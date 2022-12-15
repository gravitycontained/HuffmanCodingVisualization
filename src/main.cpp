#include <qpl/qpl.hpp>



std::string random_string(qpl::size size) {

	auto vec = qpl::vector_0_to_n(127 - 32, 32);

	std::string result;
	while (result.length() < size) {
		qpl::shuffle(vec);
		for (auto& i : vec) {
			auto factor = qpl::random(0.3, 1.0 / 0.3);
			if (qpl::random_b(0.05)) {
				factor *= 5;
			}
			auto count = (size / 100.0) * (factor * factor);
			result += std::string(qpl::random(qpl::size_cast(count)), qpl::char_cast(i));
		}
	}

	qpl::shuffle(result);

	result.resize(size);
	return result;
}

struct tree_node {

	tree_node() {
		this->line_a.set_color(qpl::rgb::grey_shade(200));
		this->line_b.set_color(qpl::rgb::grey_shade(200));

		this->text_a.set_font("consola");
		this->text_b.set_font("consola");
		this->text_a.set_character_size(300);
		this->text_b.set_character_size(300);


		this->text_a_background.set_color(qpl::rgb::grey_shade(40));
		this->text_a_background.set_slope_dimension({ 20, 20 });
		this->text_b_background.set_color(qpl::rgb::grey_shade(40));
		this->text_b_background.set_slope_dimension({ 20, 20 });
	}

	void set_position(qpl::vector2f position) {
		this->position = position;
		this->line_a.set_a(position);
		this->line_a.set_b(position + qpl::vec(-this->delta.x, this->delta.y));
		this->line_a.set_thickness(15);
		this->line_b.set_a(position);
		this->line_b.set_b(position + this->delta);
		this->line_b.set_thickness(15);

		this->line_a.extend_a(-5);
		this->line_b.extend_a(5);
		this->active = true;
	}

	void find_char_get_encoding_helper(char c, bool& found, std::string& encoding, std::string path) {
		if (found) {
			return;
		}
		if (this->text_a.get_string() == qpl::to_string(c)) {
			encoding = path + '0';
			found = true;
			return;
		}
		if (this->text_b.get_string() == qpl::to_string(c)) {
			encoding = path + '1';
			found = true;
			return;
		}
		if (this->nodes.size()) {
			encoding += '0';
			this->nodes[0u].find_char_get_encoding_helper(c, found, encoding, path + '0');

			encoding += '1';
			this->nodes[1u].find_char_get_encoding_helper(c, found, encoding, path + '1');
		}
	}
	void mark_found_path(const std::string& path, qpl::size index, qpl::rgb path_color) {
		if (path[index] == '0') {
			this->line_a.set_color(path_color);
			if (this->nodes.size()) {
				this->nodes[0u].mark_found_path(path, index + 1, path_color);
			}
		}
		else if (path[index] == '1') {
			this->line_b.set_color(path_color);
			if (this->nodes.size()) {
				this->nodes[1u].mark_found_path(path, index + 1, path_color);
			}
		}
	}
	void reset_path_colors() {
		this->line_a.set_color(qpl::rgb::grey_shade(200));
		this->line_b.set_color(qpl::rgb::grey_shade(200));

		if (this->nodes.size()) {
			this->nodes[0u].reset_path_colors();
			this->nodes[1u].reset_path_colors();
		}
	}

	std::string find_char_get_encoding(char c, qpl::rgb path_color) {
		this->reset_path_colors();

		std::string encoding = "";
		bool found = false;
		this->find_char_get_encoding_helper(c, found, encoding, "");

		this->mark_found_path(encoding, 0u, path_color);

		return encoding;
	}

	qpl::vector2f get_lower_left() const {
		return this->position + qpl::vec(-this->delta.x, this->delta.y);
	}
	qpl::vector2f get_lower_right() const {
		return this->position + this->delta;
	}
	void move(qpl::vector2f diff) {
		if (!this->active) {
			return;
		}
		this->position.move(diff);
		this->line_a.move(diff);
		this->text_a.move(diff);
		this->text_a_background.move(diff);
		this->line_b.move(diff);
		this->text_b.move(diff);
		this->text_b_background.move(diff);

		if (this->nodes.size()) {
			this->nodes[0u].move(diff);
			this->nodes[1u].move(diff);
		}
	}

	void draw(qsf::draw_object& draw) const {
		if (!this->active) {
			return;
		}
		draw.draw(this->line_a);
		draw.draw(this->line_b);

		if (!this->text_a.get_string().empty()) {
			draw.draw(this->text_a_background);
			draw.draw(this->text_a);
		}
		if (!this->text_b.get_string().empty()) {
			draw.draw(this->text_b_background);
			draw.draw(this->text_b);
		}

		draw.draw(this->nodes);
	}
	void most_left_helper(qpl::vector2f& value) const {
		if (this->position != qpl::vec(0, 0)) {
			if (this->get_lower_left().x < value.x) {
				value = this->get_lower_left();
			}
		}
		for (auto& i : this->nodes) {
			i.most_left_helper(value);
		}
	}
	qpl::vector2f most_left_position() const {
		qpl::vector2f value = qpl::vec(qpl::f32_max, qpl::f32_max);
		this->most_left_helper(value);
		return value;
	}

	void set(std::string left, std::string right) {
		this->text_a.set_string("");
		this->text_b.set_string("");
		if (left.length() == 1u) {
			this->text_a.set_string(qpl::to_string(left));
			this->text_a.set_center(this->get_lower_left());
			this->text_a_background.set_hitbox(this->text_a.get_visible_hitbox().increased(30));
		}
		if (right.length() == 1u) {
			this->text_b.set_string(qpl::to_string(right));
			this->text_b.set_center(this->get_lower_right());
			this->text_b_background.set_hitbox(this->text_b.get_visible_hitbox().increased(30));
		}
	}

	qsf::thick_line line_a;
	qsf::thick_line line_b;

	qsf::text text_a;
	qsf::text text_b;

	qsf::smooth_rectangle text_a_background;
	qsf::smooth_rectangle text_b_background;

	qpl::vector2f position;
	qpl::vector2f delta = qpl::vec(150, 600);

	std::vector<tree_node> nodes;
	bool active = false;
};

struct main_state : qsf::base_state {

	void make_string() {
		this->string = random_string(20'000u);
		//this->string = "AN_ASSASSIN_SINS";

		auto split = qpl::split_string_every(this->string, this->original_string_split_size);
		this->string.clear();
		for (auto& i : split) {
			this->string += i + "\n";
		}
		this->original_text.set_string(this->string);
		this->original_text_background.set_hitbox(this->original_text.get_visible_hitbox().increased(100));

		this->randomize_button.set_center(this->original_text_background.get_center());
		this->randomize_button.set_position(this->randomize_button.get_position().with_y(-500));
		this->randomize_button.centerize_text();

		this->count_button.set_position({ this->original_text_background.get_dimension().x + 50, -500 });
		this->count_button.centerize_text();

		this->character_table_visible = false;
		this->build_tree_visible = false;
		this->make_binary_encoding_visible = false;
		this->nodes.clear();

		this->original_text_hitbox = this->original_text.get_all_characters_hitbox_whitespace_included();
	}

	void update_counts() {
		qpl::sort(this->counts, [](const auto& a, const auto& b) { return a.second < b.second; });

		qpl::size max = 0u;
		for (auto& i : counts) {
			max = qpl::max(max, i.first.length());
		}

		this->character_table_text.clear();

		std::string string;
		for (auto& i : counts) {
			auto add = std::string((max + 2) - qpl::to_string("'", i.first, "'").length(), ' ');

			this->character_table_text << "'";

			for (auto& c : i.first) {
				auto color = this->character_color_map[qpl::to_string(c)];
				this->character_table_text << qsf::ts::push() << qsf::ts::color(color) << c << qsf::ts::pop();
			}
			this->character_table_text << "'" << add << " x " << i.second << '\n';
		}

		this->character_table_text.set_position({ this->original_text_background.get_dimension().x + 300, 0 });
		this->character_table_background.set_hitbox(this->character_table_text.get_visible_hitbox().increased(100));
	}

	void count() {

		std::unordered_map<char, qpl::size> count_map;
		for (auto& i : this->string) {
			if (count_map.find(i) == count_map.cend()) {
				count_map[i] = 0u;
			}
			++count_map[i];
		}

		this->nodes.clear();
		this->counts.clear(); 
		this->character_encoding.clear();
		this->compressed_text.clear();

		for (auto& i : count_map) {
			if (i.first != '\n') {
				this->counts.push_back(std::make_pair(qpl::to_string(i.first), i.second));
				this->character_color_map[qpl::to_string(i.first)] = qpl::get_rainbow_color(qpl::random(0.0, 1.0));
			}
		}
		this->update_counts();
		this->character_table_visible = true;
		this->make_binary_encoding_visible = false;
		this->compress_visible = false;
		this->compressing = false;
		this->compressed_text_visible = false;
		this->result_visible = false;


		this->build_tree_button.set_position({ this->original_text_background.get_dimension().x + 50 + 1350, -500 });
		this->build_tree_button.centerize_text();

		this->make_binary_encoding_button.set_position({ this->original_text_background.get_dimension().x + 50 + 1350 * 2, -500 });
		this->make_binary_encoding_button.centerize_text();

		this->compress_button.set_position({ this->original_text_background.get_dimension().x + 50 + 1350 * 3, -500 });
		this->compress_button.centerize_text();
	}

	void build_step() {
		if (this->counts.size() <= 1u) {
			this->building_tree = false;
			this->final_tree_characters = this->counts.front().first;
			return;
		}
		auto offset = qpl::vec(this->original_text_background.get_dimension().x + 2000, 400);

		auto x = (this->nodes.size()) % 8;
		auto y = (this->nodes.size()) / 8;


		auto add = qpl::vec(x, y) * qpl::vec(1500, 3600);

		auto str = qpl::to_string(this->counts[0].first, this->counts[1].first);

		this->nodes[str].set_position(offset + add);
		this->nodes[str].set(this->counts[0].first, this->counts[1].first);
		
		qpl::rgb color = qpl::rgb::white;
		if (this->character_color_map.find(this->counts[0].first) != this->character_color_map.cend()) {
			color = this->character_color_map[this->counts[0].first];
		}
		this->nodes[str].text_a.set_color(color);
		color = qpl::rgb::white;
		if (this->character_color_map.find(this->counts[1].first) != this->character_color_map.cend()) {
			color = this->character_color_map[this->counts[1].first];
		}
		this->nodes[str].text_b.set_color(color);


		auto left_big = this->counts[0].first.length() > 1u;
		auto right_big = this->counts[1].first.length() > 1u;

		if (left_big && right_big) {
			this->nodes[str].delta = this->nodes[this->counts[0].first].delta;
			this->nodes[str].delta.x += this->nodes[this->counts[1].first].delta.x * 1.5;
			this->nodes[str].delta.y *= 1.2;
			auto pos = this->nodes[str].position;
			this->nodes[str].set_position(pos);
			this->nodes[str].set(this->counts[0].first, this->counts[1].first);
		}

		if (left_big) {
			auto diff = this->nodes[str].get_lower_left() - this->nodes[this->counts[0].first].position;
			this->nodes[this->counts[0].first].move(diff);
			if (this->nodes[str].nodes.empty()) {
				this->nodes[str].nodes.resize(2u);
			}
			this->nodes[str].nodes[0] = this->nodes[this->counts[0].first];
			this->nodes.erase(this->nodes.find(this->counts[0].first));

		}
		if (right_big) {
			auto diff = this->nodes[str].get_lower_right() - this->nodes[this->counts[1].first].position;
			this->nodes[this->counts[1].first].move(diff);
			if (this->nodes[str].nodes.empty()) {
				this->nodes[str].nodes.resize(2u);
			}
			this->nodes[str].nodes[1] = this->nodes[this->counts[1].first];
			this->nodes.erase(this->nodes.find(this->counts[1].first));
		}

		this->counts[0].first = str;
		this->counts[0].second = this->counts[0].second + this->counts[1].second;
		std::swap(this->counts[1], this->counts.back());
		this->counts.pop_back();

		this->update_counts();

		if (this->counts.size() <= 1u) {
			auto value = this->nodes[str].most_left_position();


			auto diff_x = (this->original_text_background.get_position().x + this->original_text_background.get_dimension().x) - value.x + 3000;
			auto diff_y = -this->nodes[str].position.y;
			this->nodes[str].move(qpl::vec(diff_x, diff_y));

			this->make_binary_encoding_visible = true;
		}
	}

	void build_tree() {
		this->build_tree_visible = true;
		this->building_tree = true;
	}

	void encoding_step() {
		if (this->character_encoding.size() == this->final_tree_characters.size()) {
			this->making_encoding = false;
			this->nodes.begin()->second.reset_path_colors();
			this->compress_visible = true;
			return;
		}

		auto c = this->final_tree_characters[this->encoding_progress_index];
		auto info = this->nodes.begin()->second.find_char_get_encoding(c, this->character_color_map[qpl::to_string(c)]);
		this->character_encoding[qpl::to_string(c)] = info;

		this->make_encoding_text_stream();

		++this->encoding_progress_index;
	}

	void make_encoding_text_stream() {
		this->character_table_text.clear();
		std::string string;
		for (auto& i : this->final_tree_characters) {
			this->character_table_text << "'";

			auto s = qpl::to_string(i);

			auto color = this->character_color_map[s];
			this->character_table_text << qsf::ts::push() << qsf::ts::color(color) << i << qsf::ts::pop();
			this->character_table_text << "'";

			if (this->character_encoding.find(s) != this->character_encoding.cend()) {
				this->character_table_text << " = ";
				auto binary_string = this->character_encoding[s];

				this->character_table_text << qsf::ts::push();
				for (auto& b : binary_string) {
					if (b == '0') {
						this->character_table_text << qsf::ts::color(qpl::rgb(100, 255, 100)) << b;
					}
					else if (b == '1') {
						this->character_table_text << qsf::ts::color(qpl::rgb(255, 100, 100)) << b;
					}
				}
				this->character_table_text << qsf::ts::pop();
			}

			this->character_table_text << '\n';
		}

		this->character_table_text.set_position({ this->original_text_background.get_dimension().x + 300, 0 });
		this->character_table_background.set_hitbox(this->character_table_text.get_visible_hitbox().increased(100));
	}

	void make_encoding() {
		this->make_encoding_text_stream();

		this->making_encoding = true; 
		this->encoding_progress_index = 0u;
	}

	void compress() {
		this->compressing = true;
		this->compressed_text_visible = true;
		this->compressing_progress_index = 0u;
		this->compressing_character_count = 0u;
		this->final_compressed_string.clear();

		auto h1 = this->character_table_text.get_visible_hitbox();
		auto h2 = this->original_text.get_visible_hitbox();
		auto max = qpl::max(h1.position.y + h1.dimension.y, h2.position.y + h2.dimension.y);
		this->compressed_text.set_position({ 0, max + 1000 });
	}

	void make_compress_progress() {
		this->compressed_progress_rects.clear();

		qpl::size starting_index = 0u;
		if (this->compressing_progress_index > this->original_string_split_size) {

			this->compressed_progress_rects.push_back({});
			auto starting_pos = this->original_text_hitbox.front().get_position();

			auto row = (this->compressing_progress_index / this->original_string_split_size) * this->original_string_split_size;
			auto target = this->original_text_hitbox[row - 1];

			this->compressed_progress_rects.back().set_position(starting_pos);
			this->compressed_progress_rects.back().set_dimension((target.position + target.dimension) - starting_pos);
			this->compressed_progress_rects.back().set_color(qpl::rgb::green().with_alpha(50));
			starting_index = row;
		}

		this->compressed_progress_rects.push_back({});

		auto starting_pos = this->original_text_hitbox[starting_index].get_position();
		auto target = this->original_text_hitbox[this->compressing_progress_index];
		this->compressed_progress_rects.back().set_position(starting_pos);
		this->compressed_progress_rects.back().set_dimension((target.position + target.dimension) - starting_pos);
		this->compressed_progress_rects.back().set_color(qpl::rgb::green().with_alpha(50));
	}

	void compress_step() {
		if (this->compressing_progress_index == this->string.length()) {
			this->compressing = false;
			this->result_visible = true;

			return;
		}
		auto c = this->string[this->compressing_progress_index];
		auto bin = this->character_encoding[qpl::to_string(c)];

		this->final_compressed_string += bin;

		for (auto& b : bin) {
			this->compressed_line += b;
			if (this->compressing_character_count && this->compressing_character_count % 500 == 0u) {
				if (this->compressing_character_count > 500) {
					this->compressed_text << '\n';
				}
				this->compressed_text << compressed_line;
				this->compressed_line.clear();
			}
			++this->compressing_character_count;
		}
		
		this->make_compress_progress();
		this->compressed_text_background.set_hitbox(this->compressed_text.get_visible_hitbox().increased(100));

		++this->compressing_progress_index;

		if (this->compressing_progress_index == this->string.length()) {
			this->result_text.set_position(this->compressed_text.get_visible_hitbox().position + qpl::vec(5000, this->compressed_text.get_visible_hitbox().dimension.y + 1000));

			if (this->compressed_line.length()) {
				if (this->compressing_character_count > 500) {
					this->compressed_text << '\n';
				}
				this->compressed_text << compressed_line;
				this->make_compress_progress();
				this->compressed_text_background.set_hitbox(this->compressed_text.get_visible_hitbox().increased(100));
			}

			this->compressed_progress_rects.clear();

			auto original = (this->string.length() * 8);
			auto diff = qpl::f64_cast(this->final_compressed_string.length()) / original;

			std::string result;
			result += qpl::to_string("original string   = ", original, " bits\n");
			result += qpl::to_string("compressed string = ", this->final_compressed_string.length(), " bits\n");

			auto times = (1.0 - diff) * 100;
			result += qpl::to_string("-", qpl::to_string_precision(2, times), "% size");

			this->result_text.set_string(result);
			this->result_text_background.set_hitbox(this->result_text.get_visible_hitbox().increased(400));
		}

	}

	void init() override {
		this->call_on_resize();

		this->original_text.set_font("consola");
		this->original_text.set_character_size(100u);

		this->compressed_text.set_font("consola");
		this->compressed_text.set_character_size(100u);

		this->result_text.set_font("consola");
		this->result_text.set_character_size(500u);

		this->randomize_button.set_dimension({ 1100, 300 });
		this->randomize_button.set_background_color(qpl::rgb::grey_shade(40));
		this->randomize_button.set_background_slope_dimension({ 100, 100 });
		this->randomize_button.set_text_font("consola");
		this->randomize_button.set_text_string("randomize");
		this->randomize_button.set_text_character_size(200u);
		this->randomize_button.set_text_style(sf::Text::Style::Bold);

		this->count_button = this->randomize_button;
		this->count_button.set_text_string("count");

		this->build_tree_button = this->randomize_button;
		this->build_tree_button.set_text_string("build");

		this->make_binary_encoding_button = this->randomize_button;
		this->make_binary_encoding_button.set_text_string("encoding");

		this->compress_button = this->randomize_button;
		this->compress_button.set_text_string("compress");

		this->character_table_text.set_font("consola");
		this->character_table_text.set_character_size(100u);

		this->original_text_background.set_slope_dimension({ 100, 100 });
		this->original_text_background.set_color(qpl::rgb::grey_shade(40));

		this->character_table_background.set_color(qpl::rgb::grey_shade(40));
		this->character_table_background.set_slope_dimension({ 100, 100 });

		this->compressed_text_background.set_color(qpl::rgb::grey_shade(40));
		this->compressed_text_background.set_slope_dimension({ 100, 100 });

		this->result_text_background.set_color(qpl::rgb::grey_shade(40));
		this->result_text_background.set_slope_dimension({ 500, 500 });


		this->make_string();

	}
	void call_on_resize() override {
		this->view.set_hitbox(*this);
	}
	void updating() override {
		this->update(this->view);
		this->update(this->randomize_button, this->view);
		this->update(this->count_button, this->view);
		this->update(this->build_tree_button, this->view);
		this->update(this->make_binary_encoding_button, this->view);
		this->update(this->compress_button, this->view);

		if (this->randomize_button.is_clicked()) {
			this->make_string();
		}
		if (this->count_button.is_clicked()) {
			this->count();
		}
		if (this->build_tree_button.is_clicked()) {
			if (this->nodes.empty()) {
				this->build_tree();
			}
		}
		if (this->make_binary_encoding_button.is_clicked() && this->make_binary_encoding_visible) {
			this->make_encoding();
		}
		if (this->compress_button.is_clicked() && this->compress_visible) {
			this->compress();
		}

		auto delta = 1.0;
		if (this->event().key_holding(sf::Keyboard::LShift)) {
			delta = 10.0;
		}

		if (this->building_tree) {


			if (this->build_tree_clock.has_elapsed_reset(0.05 * delta)) {
				this->build_step();
			}
		}
		if (this->making_encoding) {
			if (this->make_encoding_clock.has_elapsed_reset(0.05 * delta)) {
				this->encoding_step();
			}
		}
		if (this->compressing) {
			if (this->compressing_clock.has_elapsed_reset(0.02 * delta)) {
				for (qpl::size i = 0u; i < 50u; ++i) {
					this->compress_step();
				}
			}
		}
		
	}
	void drawing() override {
		this->draw(this->original_text_background, this->view);
		this->draw(this->original_text, this->view);

		if (this->character_table_visible) {
			this->draw(this->character_table_background, this->view);
			this->draw(this->character_table_text, this->view);
			this->draw(this->build_tree_button, this->view);
		}
		this->draw(this->randomize_button, this->view);
		this->draw(this->count_button, this->view);

		if (this->build_tree_visible) {
			for (auto& i : this->nodes) {
				this->draw(i.second, this->view);
			}
		}
		if (this->make_binary_encoding_visible) {
			this->draw(this->make_binary_encoding_button, this->view);
		}
		if (this->compress_visible) {
			this->draw(this->compress_button, this->view);
		}
		if (this->compressed_text_visible) {
			this->draw(this->compressed_text_background, this->view);
			this->draw(this->compressed_text, this->view);
			this->draw(this->compressed_progress_rects, this->view);
		}
		if (this->result_visible) {
			this->draw(this->result_text_background, this->view);
			this->draw(this->result_text, this->view);
		}

		//for (auto& i : this->original_text_hitbox) {
		//	qsf::rectangle rect;
		//	rect.set_hitbox(i);
		//	rect.set_color(qpl::rgba::transparent());
		//	rect.set_outline_color(qpl::rgb::red());
		//	rect.set_outline_thickness(2.0f);
		//	this->draw(rect, this->view);
		//}
	}

	std::string string;

	qsf::text original_text;
	qsf::text compressed_text;
	qsf::text result_text;
	qsf::text_stream character_table_text;
	qsf::view_rectangle view;
	qsf::smooth_button randomize_button;
	qsf::smooth_button count_button;
	qsf::smooth_button build_tree_button;
	qsf::smooth_button make_binary_encoding_button;
	qsf::smooth_button compress_button;
	qsf::smooth_rectangle original_text_background;
	qsf::smooth_rectangle compressed_text_background;
	qsf::smooth_rectangle character_table_background;
	qsf::smooth_rectangle result_text_background;
	std::vector<std::pair<std::string, qpl::size>> counts;
	std::string final_tree_characters;
	std::string final_compressed_string;
	std::string compressed_line;
	std::vector<qpl::hitbox> original_text_hitbox;

	std::vector<qsf::rectangle> compressed_progress_rects;

	std::unordered_map<std::string, tree_node> nodes;
	std::unordered_map<std::string, qpl::rgb> character_color_map;
	std::unordered_map<std::string, std::string> character_encoding;

	bool character_table_visible = false;
	bool build_tree_visible = false;
	bool make_binary_encoding_visible = false;
	bool building_tree = false;
	bool making_encoding = false;
	bool compress_visible = false;
	bool compressed_text_visible = false;
	bool compressing = false;
	bool result_visible = false;

	qpl::size encoding_progress_index = 0u;
	qpl::size compressing_progress_index = 0u;
	qpl::size compressing_character_count = 0u;
	qpl::size original_string_split_size = 240u;

	qpl::small_clock build_tree_clock;
	qpl::small_clock make_encoding_clock;
	qpl::small_clock compressing_clock;
};

int main() try {
	qsf::framework framework;
	framework.set_title("QPL");
	framework.add_font("consola", "resources/consola.ttf");
	framework.set_dimension({ 1400u, 950u });

	framework.add_state<main_state>();
	framework.game_loop();
}
catch (std::exception& any) {
	qpl::println("caught exception:\n", any.what());
	qpl::system_pause();
}