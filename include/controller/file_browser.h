#pragma once

//local
#include "states.h"
#include "app/exchange_data.h"
#include "app/env.h"

//framework
#include <dfw/controller_interface.h>

//logger
#include <lm/logger.h>
#include <ldtools/view_composer.h>
#include <ldtools/ttf_manager.h>
#include <tools/pager.h>

//std
#include <tools/file_utils.h>

namespace controller {

class file_browser:
	public dfw::controller_interface {

	public:

	                            file_browser(lm::logger&, ldtools::ttf_manager&, tile_editor::exchange_data&, const tile_editor::env&, int);
	virtual void                loop(dfw::input&, const dfw::loop_iteration_data&);
	virtual void                draw(ldv::screen&, int);
	virtual void                awake(dfw::input& /*input*/, int=-1);
	virtual void                slumber(dfw::input& /*input*/, int) {}
	virtual bool                can_leave_state() const {return true;}

	private:

	enum class working_modes{navigate, create};

	struct entry {
		enum class entry_type {file, dir, create};
		std::string             path_name;
		entry_type              type;
		bool                    is_dir() const {return entry_type::dir==type;}
		bool                    is_new() const {return entry_type::create==type;}
		bool                    is_file() const {return entry_type::file==type;}
		bool                    operator<(const entry& _other) const {

			if(_other.is_new() && !is_new()) {
				return false;
			}

			if(is_new() && !_other.is_new()) {
				return true;
			}

			if(_other.is_dir() && !is_dir()) {
				return false;
			}

			if(is_dir() && !_other.is_dir()) {
				return true;
			}

			return path_name < _other.path_name;
		}
	};

	//!Does the navigation input.
	void                        input_navigation(dfw::input&);
	//!Does the input for new names.
	void                        input_create(dfw::input&);
	//!Fills up "contents" with the contents of the current directory.
	void						extract_entries();
	//!Refreshes the ttf_representation.
	void						refresh_list_view();
	//!Positions the current selector representation.
	void                        position_selector();
	//!Void composes the title
	void                        compose_title();
	//!Void sets the exchange data...
	void                        solve(bool, const std::string&);


	//references...
	lm::logger&                 log;
	ldtools::ttf_manager&       ttf_manager;
	tile_editor::exchange_data& exchange_data;

	//properties
	working_modes				mode;
	tools::filesystem::path     current_directory;
	std::string                 title;
	std::vector<entry>          contents;
	int                         first_selection_y{0},
	                            y_selection_factor{0};
	bool                        allow_create{true}; //!< True if it allows the [new] entry.
	int                         invoker_id{0}; //!< Stores the id of the invoking controller so packages can be left for it.

	tools::pager                pager;

	//view properties.
	ldtools::view_composer		layout;

};

}
