#pragma once

//local
#include "states.h"
#include "app/env.h"

#include <dfw/controller_interface.h>
#include <ldv/ttf_representation.h>
#include <ldv/camera.h>
#include <ldtools/ttf_manager.h>
#include <lm/logger.h>
#include <vector>

namespace controller {

class help:
	public dfw::controller_interface {

	public:

								help(lm::logger&, ldtools::ttf_manager&, const tile_editor::env&, unsigned int, unsigned int);
	virtual void 				loop(dfw::input&, const dfw::loop_iteration_data&);
	virtual void 				draw(ldv::screen&, int);
	virtual void 				awake(dfw::input& /*input*/, int=-1);
	virtual void 				slumber(dfw::input& /*input*/, int) {}
	virtual bool				can_leave_state() const {return true;}

	private:

	//references...
	lm::logger&                 log;
	ldtools::ttf_manager&       ttf_manager;

	//properties
	ldv::camera                 camera;
	ldv::ttf_representation     help_rep;

	struct help_section {

		std::string             title,
		                        text;

		void clear() {

			title="";
			text="";
		}
	};

	std::vector<help_section>   sections;
	std::size_t                 section_index=0;

	void                        ready_help(const std::string&);
	void                        next_section();
	void                        previous_section();
	void                        ready_section(std::size_t);
};

}
