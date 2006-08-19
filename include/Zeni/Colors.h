/* This file is part of the Zenipex Library.
* Copyleft (C) 2006 Mitchell Keith Bloch a.k.a. bazald
*
* The Zenipex Library is free software; you can redistribute it and/or 
* modify it under the terms of the GNU General Public License as 
* published by the Free Software Foundation; either version 2 of the 
* License, or (at your option) any later version.
*
* The Zenipex Library is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License 
* along with the Zenipex Library; if not, write to the Free Software 
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 
* 02110-1301 USA.
*
* As a special exception, you may use this file as part of a free software
* library without restriction.  Specifically, if other files instantiate
* templates or use macros or inline functions from this file, or you compile
* this file and link it with other files to produce an executable, this
* file does not by itself cause the resulting executable to be covered by
* the GNU General Public License.  This exception does not however
* invalidate any other reasons why the executable file might be covered by
* the GNU General Public License.
*/

/**
 * \class Zeni::Colors
 *
 * \ingroup Zenilib
 *
 * \brief A Color database read in from a file.
 *
 * One can retrieve colors by name from this singleton class.
 *
 * \note It is more useful to create names based on intended use of a color than it is to create names based on the color itself.
 *
 * \author bazald
 *
 * Contact: bazald@zenipex.com
 */

#ifndef ZENI_COLORS_H
#define ZENI_COLORS_H

#include "Color.h"
#include "Core.h"

#include <map>
#include <string>

namespace Zeni {

  class Colors {
    Colors();

    // Undefined
    Colors(const Colors &);
    Colors & operator=(const Colors &);

  public:
    // Get reference to only instance;
    static Colors & get_reference(); ///< Get access to the singleton.

    inline Color get_color(const std::string &color) const; ///< Get a color by name.

    // May throw Colors_Init_Failure
    inline void set_color(const std::string &name, const Color &color); ///< Set a color by name.
    void reload(const std::string &colors = ""); ///< Reload the database or choose a new one.

  private:
    void init();
    const Color & access_color(const std::string &color) const;

    std::string m_colordb;
    std::map <std::string, Color> m_color;
  };

  struct Color_Not_Found : public Error {
    Color_Not_Found() : Error("Zeni Color Not Found") {}
  };

  struct Colors_Init_Failure : public Error {
    Colors_Init_Failure() : Error("Zeni Colors Failed to Initialize Correctly") {}
  };

}

#ifdef ZENI_INLINES
#include "Colors.hxx"
#endif

#endif
