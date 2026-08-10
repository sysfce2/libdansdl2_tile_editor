#pragma once

//local
#include "states.h"
#include "app/exchange_data.h"
#include "editor_types/property_manager.h"
#include "blueprint_types/property_table.h"

#include <dfw/controller_interface.h>
#include <ldtools/ttf_manager.h>
#include <lm/logger.h>
#include <vector>
#include <sstream>
#include <type_traits>

namespace controller {

//!Controller to modify property tables.
class properties:
	public dfw::controller_interface {

	public:

								properties(lm::logger&, ldtools::ttf_manager&, tile_editor::exchange_data&);
	virtual void 				loop(dfw::input&, const dfw::loop_iteration_data&);
	virtual void 				draw(ldv::screen&, int);
	virtual void 				awake(dfw::input& /*input*/, int=-1);
	virtual void 				slumber(dfw::input& /*input*/, int);
	virtual bool				can_leave_state() const {return true;}
	void                        request_draw(dfw::controller_view_manager&);

	private:

	void                        input_text(dfw::input&);
	void                        input_traverse(dfw::input&);
	void                        save_changes();
	void                        cancel_changes();

	//inner printer class.
	struct printer {

		const int index{0};
		const int max_index{0};
		const std::string& edit_value;
		const bool text_mode;
		std::stringstream ss;

		template<typename B, typename T>
		void print(const B& _blueprint, const T& _value, const std::string& _type) {

			ss<<(index+1)<<" / "<<max_index<<": "<<_blueprint.name<<" ("<<_type<<"):";

			if(text_mode) {
				ss<<edit_value;
			}
			else {
				ss<<_value;
			}

			ss<<std::endl
				<<"\t"<<_blueprint.description;

			switch(_blueprint.linked_to) {
				case tile_editor::property_links::nothing:break;
				case tile_editor::property_links::w: ss<<" [this property is linked to entity witdh]"; break;
				case tile_editor::property_links::h: ss<<" [this property is linked to entity height]"; break;
				case tile_editor::property_links::color_red: ss<<" [this property is linked to entity red color component]"; break;
				case tile_editor::property_links::color_green: ss<<" [this property is linked to entity green color component]"; break;
				case tile_editor::property_links::color_blue: ss<<" [this property is linked to entity blue color component]"; break;
				case tile_editor::property_links::color_alpha: ss<<" [this property is linked to entity color alpha]"; break;
			}

			ss<<std::endl;
		}

		void special(const std::string& _type) {

			ss<<(index+1)<<" / "<<max_index<<": "<<_type<<std::endl;
		}
	};

	//references...
	lm::logger&                 log;
	ldtools::ttf_manager&       ttf_manager;
	tile_editor::exchange_data& exchange_data;

	//properties
	tile_editor::property_manager   property_manager_backup; //!<Backup of the input data: changes are made live.
	tile_editor::property_manager * property_manager{nullptr}; //!<Data changed on real time.
	tile_editor::property_table *   blueprint{nullptr};

	enum class option_types{str, integer, decimal};
	struct option_lookup {
		option_types                type;
		std::string                 key;
	};

	std::vector<option_lookup>      lookup;
	int                             current_index{0},
	                                exit_index{0},
	                                cancel_index{0};
	std::string                     current_value;
	bool                            text_mode{false};
};

}
