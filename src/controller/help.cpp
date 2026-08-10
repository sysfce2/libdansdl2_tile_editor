#include "controller/help.h"
#include "input/input.h"
#include "app/definitions.h"

#include <tools/file_utils.h>

using namespace controller;

help::help(
	lm::logger& _log,
	ldtools::ttf_manager& _ttf_manager,
	const tile_editor::env& _env,
	unsigned int _w,
	unsigned int _h
)
	:log(_log),
	ttf_manager{_ttf_manager},
	camera{
		{0,0, _w, _h},
		{0,0}
	},
	help_rep{
		ttf_manager.get(tile_editor::definitions::main_font_name, tile_editor::definitions::main_font_size),
		ldv::rgba8(255, 255, 255, 255),
	} {

	help_rep.set_max_width(_w);

	//My computers don't have enough juice for large textures, so split the
	//text.
	std::string helpstr=tools::dump_file(_env.build_data_path("help.txt"));
	ready_help(helpstr);
	ready_section(section_index);
}


void help::awake(
	dfw::input&,
	int
) {

	camera.go_to({0,0});
}

void help::loop(dfw::input& _input, const dfw::loop_iteration_data& /*lid*/) {

	if(_input().is_exit_signal()) {
		set_leave(true);
		return;
	}

	if(_input.is_input_down(input::escape)) {
		pop_state();
		return;
	}

	if(_input.is_input_pressed(input::down)) {

		camera.move_by(0, 5);
	}
	else if(_input.is_input_pressed(input::up)) {

		camera.move_by(0, -5);
	}
	else if(_input.is_input_down(input::left)) {

		previous_section();
	}
	else if(_input.is_input_down(input::right)) {

		next_section();
	}
}

void help::draw(ldv::screen& _screen, int /*fps*/) {

	_screen.clear(ldv::rgba8(0, 0, 0, 255));
	help_rep.draw(_screen, camera);
}

void help::ready_help(
	const std::string& _helpstr
) {

/**
help file is formated as such:
== TITLE ==
text until another title is found...

So basically we can read each line and do our stuff.
*/

	std::stringstream ss(_helpstr);

	help_section cursection;
	std::string line;
	while(true) {

		if(ss.eof()) {

			break;
		}

		std::getline(ss, line);
		if(!line.size()) {

			continue;
		}

		//a title...
		if("=="==line.substr(0, 2)) {

			if(cursection.title.size()) {

				sections.push_back(cursection);
			}

			cursection.clear();
			cursection.title=line;
		}
		//not a title
		else {

			cursection.text+=line+"\n";
		}
	}

	//add the last one
	sections.push_back(cursection);


	//now, insert a generic index at the beginning...
	cursection.clear();
	cursection.title="= HELP INDEX =";
	for(const auto& section: sections) {

		cursection.text+=section.title+"\n";
	}

	sections.insert(std::begin(sections), cursection);
}

void help::next_section() {

	++section_index;
	if(section_index >= sections.size()) {

		section_index=0;
	}

	ready_section(section_index);
}

void help::previous_section() {

	if(section_index==0) {

		section_index=sections.size()-1;
		ready_section(section_index);
		return;
	}

	--section_index;
	ready_section(section_index);
}

void help::ready_section(
	std::size_t _index
) {

	const auto& section=sections.at(_index);

	std::stringstream ss;
	ss<<section.title<<"\n"<<section.text;

	help_rep.set_text(ss.str());
	help_rep.go_to({0,0});
	camera.set_limits(help_rep.get_view_position());
	camera.go_to({0,0});
}
