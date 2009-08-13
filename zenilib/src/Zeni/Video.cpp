/* This file is part of the Zenipex Library.
* Copyleft (C) 2008 Mitchell Keith Bloch a.k.a. bazald
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

#include <Zeni/Video.hxx>

#include <Zeni/Camera.hxx>
#include <Zeni/Coordinate.hxx>
#include <Zeni/Matrix4f.hxx>
#include <Zeni/Vector3f.hxx>
#include <Zeni/Video_GL.h>
#include <Zeni/Video_DX9.h>

#ifdef _MACOSX
#include <SDL_image/SDL_image.h>
#else
#include <SDL/SDL_image.h>
#endif

#include <iostream>

using namespace std;

namespace Zeni {

  Video::Video(const Video_Base::VIDEO_MODE &vtype_)
    : Video_Base::IV(vtype_),
    m_display_surface(0), 
    m_icon_surface(0),
    m_opengl_flag(0), 
    m_title("Zenilib Application"), 
    m_taskmsg("Zenilib Application"), 
    m_icon("icons/icon.gif"),
    m_color(1.0f, 1.0f, 1.0f, 1.0f),
    m_clear_color(1.0f, 0.0f, 0.0f, 0.0f),
    m_preview(Matrix4f::Translate(Vector3f(-0.5f, -0.5f, 0.0f)) *
      Matrix4f::Scale(Vector3f(0.5f, -0.5f, -1.0f)) *
      Matrix4f::Translate(Vector3f(1.0f, -1.0f, 0.0f))),
    m_alpha_test(false),
    m_alpha_function(Video::ZENI_ALWAYS),
    m_alpha_value(0.0f)
  {
    if(!g_enabled)
      throw Video_Init_Failure();

    if(SDL_InitSubSystem(SDL_INIT_VIDEO) == -1)
      throw Video_Init_Failure();
  }

  Video::~Video() {
  }

  Video & get_Video() {
    if(!Video::e_video) {
      Core &cr = get_Core();

      const std::string appdata_path = cr.get_appdata_path();

      const string user_normal = appdata_path + "config/zenilib.xml";
      const string user_backup = user_normal + ".bak";
      const string local_normal = "config/zenilib.xml";
      const string local_backup = local_normal + ".bak";

      try {
        switch(Video::g_video_mode) {
        case Video_Base::ZENI_VIDEO_ANY:
#ifndef DISABLE_GL
        case Video_Base::ZENI_VIDEO_GL:
          Video::e_video = new Video_GL();
          break;
#endif
#ifndef DISABLE_DX9
        case Video_Base::ZENI_VIDEO_DX9:
          Video::e_video = new Video_DX9();
          break;
#endif
        default:
          throw Video_Init_Failure();
        }
      }
      catch(Video_Init_Failure &) {
        if(cr.copy_file(local_backup, local_normal) && cr.delete_file(local_backup))
          cerr << '\'' << local_normal << "' backup restored due to initialization failure.\n";
        if(cr.copy_file(user_backup, user_normal) && cr.delete_file(user_backup))
          cerr << '\'' << user_normal << "' backup restored due to initialization failure.\n";
        else {
          XML_Document config_xml(local_normal);

          XML_Element zenilib = config_xml["Zenilib"];
          {
            XML_Element textures = zenilib["Textures"];
            textures["Anisotropy"].set_int(0);
            textures["Bilinear_Filtering"].set_bool(true);
            textures["Mipmapping"].set_bool(true);
          }
          {
            XML_Element video = zenilib["Video"];
#ifndef DISABLE_GL
            video["API"].set_string("OpenGL");
#else
#ifndef DISABLE_DX9
            video["API"].set_string("DX9");
#endif
#endif
            video["Full_Screen"].set_bool(false);
            video["Multisampling"].set_int(0);
            {
              XML_Element screen_resolution = video["Resolution"];
              screen_resolution["Width"].set_int(800);
              screen_resolution["Height"].set_int(600);
            }
            video["Vertical_Sync"].set_bool(false);
          }

          if(cr.create_directory(appdata_path) &&
             cr.create_directory(appdata_path + "config/") &&
             config_xml.try_save(user_normal))
            cerr << '\'' << user_normal << "' reset due to initialization failure.\n";
          if(config_xml.try_save(local_normal))
            cerr << '\'' << local_normal << "' reset due to initialization failure.\n";
        }
        throw;
      }

      // Make backups of "good" configurations

      if(cr.create_directory(appdata_path) &&
         cr.create_directory(appdata_path + "config/"))
      {
        if(cr.copy_file(user_normal, user_backup))
        {
          cr.copy_file(user_normal, local_normal);
          cr.copy_file(user_normal, local_backup);
        }
        else
        {
          cr.copy_file(local_normal, user_normal);
          cr.copy_file(local_normal, user_backup);
        }
      }
      else
        cr.copy_file(local_normal, local_backup);
    }

    return *Video::e_video;
  }

  void Video::preinit_video_mode(const Video_Base::VIDEO_MODE &vm) {
    g_video_mode = vm;
  }

  void Video::preinit_screen_resolution(const Point2i &resolution) {
    g_screen_size = resolution;
  }

  void Video::preinit_full_screen(const bool &full_screen) {
    g_screen_full = full_screen;
  }

  void Video::preinit_multisampling(const int &multisampling) {
    g_multisampling = multisampling;
  }

  void Video::preinit_vertical_sync(const bool &vertical_sync) {
    g_vertical_sync = vertical_sync;
  }

  void Video::preinit_show_frame(const bool &show_frame_) {
    g_screen_show_frame = show_frame_;
  }

  void Video::reinit() {
    Vertex_Buffer::lose_all();
    get_Textures().lose_resources();
    get_Fonts().lose_resources();

    uninit();
    init();
  }

  void Video::destroy() {
    Vertex_Buffer::lose_all();
    get_Textures().lose_resources();
    get_Fonts().lose_resources();

    delete e_video;
    e_video = 0;
    g_initialized = false;
  }

  void Video::set_enabled(const bool &enabled) {
    g_enabled = enabled;
  }

  void Video::set_tt(const string &title, const string &taskmsg) {
    m_title = title;
    m_taskmsg = taskmsg;
    set_tt();
  }

  void Video::set_title(const string &title) {
    m_title = title;
    set_tt();
  }

  void Video::set_taskmsg(const string &taskmsg) {
    m_taskmsg = taskmsg;
    set_tt();
  }

  const bool Video::set_icon(const string &filename) {
    m_icon = filename;
    return set_icon();
  }

  void Video::init() {
    // Ensure Core is initialized
    get_Core();
    g_initialized = true;

    // Initialize SDL + Variables
    const SDL_VideoInfo *VideoInfo = SDL_GetVideoInfo();

    set_tt();
    set_icon();

    if(g_screen_size.x == -1)
      g_screen_size.x = 0;
    if(g_screen_size.y == -1)
      g_screen_size.y = 0;

    // Initialize Window
    m_display_surface = SDL_SetVideoMode(g_screen_size.x, g_screen_size.y, 32,
      (get_opengl_flag() ? SDL_OPENGL : 0) | 
      (g_screen_full ? SDL_FULLSCREEN : 
      (VideoInfo->wm_available && g_screen_show_frame ? 0 : SDL_NOFRAME)));

    if(!m_display_surface) {
      g_initialized = false;
      throw Video_Init_Failure();
    }

    g_screen_size.x = m_display_surface->w;
    g_screen_size.y = m_display_surface->h;
  }

  void Video::set_tt() {
    const SDL_VideoInfo *VideoInfo = SDL_GetVideoInfo();
    if(VideoInfo->wm_available)
      SDL_WM_SetCaption(m_title.c_str(), m_taskmsg.c_str());
  }

  bool Video::set_icon() {
    const SDL_VideoInfo *VideoInfo = SDL_GetVideoInfo();
    if(!VideoInfo->wm_available)
      return false;

    m_icon_surface = IMG_Load(m_icon.c_str());

    if(!m_icon_surface) {
      cerr << "Could not load display window icon\n";
      return false;
    }

    SDL_WM_SetIcon(m_icon_surface, NULL);
    return true;
  }

  Video *Video::e_video = 0;
  Video_Base::VIDEO_MODE Video::g_video_mode = Video_Base::ZENI_VIDEO_ANY;
  bool Video::g_enabled = true;
  Point2i Video::g_screen_size;
  bool Video::g_screen_full = false;
  bool Video::g_screen_show_frame = true;
  bool Video::g_initialized = false;
  bool Video::g_backface_culling = false;
  bool Video::g_lighting = false;
  bool Video::g_normal_interp = false;
  bool Video::g_vertical_sync = false;
  int Video::g_multisampling = 1;

}
