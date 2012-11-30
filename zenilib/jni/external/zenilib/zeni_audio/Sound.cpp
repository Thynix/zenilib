/* This file is part of the Zenipex Library (zenilib).
 * Copyright (C) 2011 Mitchell Keith Bloch (bazald).
 *
 * zenilib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * zenilib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with zenilib.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <zeni_audio.h>

#include <iostream>
#include <vector>
#include <iomanip>

#ifndef DISABLE_AL
#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/vorbisfile.h>
#undef OV_EXCLUDE_STATIC_CALLBACKS
#endif

#include <Zeni/Define.h>

#if defined(_DEBUG) && defined(_WINDOWS)
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include <Zeni/Singleton.hxx>

namespace Zeni {

  template class Singleton<Sound>;

  Sound * Sound::create() {
    return new Sound;
  }

  Sound::Sound()
    : m_hello_world_buffer(0),
    m_bgm(0),
    m_listener_gain(1.0f),
    m_listener_muted(false)
  {
#if !defined(DISABLE_AL)
    try {
      m_sound_renderer = new Sound_Renderer_AL;
    }
    catch(Sound_Init_Failure &) {
#endif
#if defined(ENABLE_SLES)
      try {
        m_sound_renderer = new Sound_Renderer_SLES;
      }
      catch(Sound_Init_Failure &) {
#endif
        m_sound_renderer = new Sound_Renderer_NULL;
#if defined(ENABLE_SLES)
      }
#endif
#if !defined(DISABLE_AL)
    }
#endif
  }

  Sound::~Sound() {
    delete m_bgm;
    delete m_hello_world_buffer;
    delete m_sound_renderer;
  }

  void Sound::set_listener_position(const Point3f &position) {
#ifdef DISABLE_AL
    get_BGM_Source().set_position(position);
#else
    init_BGM_Sound_Stream_AL();
    m_bgm->set_position(position);
#endif

    m_sound_renderer->set_listener_position(position);
  }

  void Sound::set_listener_velocity(const Vector3f &velocity) {
#ifdef DISABLE_AL
    get_BGM_Source().set_velocity(velocity);
#else
    init_BGM_Sound_Stream_AL();
    m_bgm->set_velocity(velocity);
#endif

    m_sound_renderer->set_listener_velocity(velocity);
  }

  void Sound::set_listener_forward_and_up(const Vector3f &forward, const Vector3f &up) {
    m_sound_renderer->set_listener_forward_and_up(forward, up);
  }

  void Sound::set_listener_gain(const float &gain) {
    m_listener_gain = gain;

    m_sound_renderer->set_listener_gain(m_listener_muted ? 0.0f : m_listener_gain);
  }

  void Sound::set_listener_muted(const bool &muted) {
    m_listener_muted = muted;

    m_sound_renderer->set_listener_gain(m_listener_muted ? 0.0f : m_listener_gain);
  }

  Point3f Sound::get_listener_position() const {
#ifdef DISABLE_AL
    return get_BGM_Source().get_position();
#else
    init_BGM_Sound_Stream_AL();
    return m_bgm->get_position();
#endif
  }

  Vector3f Sound::get_listener_velocity() const {
#ifdef DISABLE_AL
    return get_BGM_Source().get_velocity();
#else
    init_BGM_Sound_Stream_AL();
    return m_bgm->get_velocity();
#endif
  }

  std::pair<Vector3f, Vector3f> Sound::get_listener_forward_and_up() const {
    return m_sound_renderer->get_listener_forward_and_up();
  }

  void Sound::set_BGM(const String &filename) {
#ifdef DISABLE_AL
    Sound_Source &bgm_source = get_BGM_Source();

    bool playing = bgm_source.is_playing() ? true : false;

    bgm_source.stop();
    bgm_source.set_buffer(get_Hello_World_Buffer());
    delete m_bgm;
    m_bgm = 0;

    m_bgmusic = filename;
    m_bgm = new Sound_Buffer(m_bgmusic);
    bgm_source.set_buffer(*m_bgm);
    bgm_source.set_time(0.0f);

    if(playing)
      bgm_source.play();
#else
    init_BGM_Sound_Stream_AL();
    const float pitch = m_bgm->get_pitch();
    const float gain = m_bgm->get_gain();
    const Point3f position = m_bgm->get_position();
    const Vector3f velocity = m_bgm->get_velocity();
    const bool looping = m_bgm->is_looping();
    const float reference_distance = m_bgm->get_reference_distance();
    const float max_distance = m_bgm->get_max_distance();
    const float rolloff = m_bgm->get_rolloff();
    const bool playing = m_bgm->is_playing();

    delete m_bgm;
    m_bgm = 0;

    m_bgmusic = filename;
    m_bgm = new Sound_Stream_AL(filename, looping);
    m_bgm->set_pitch(pitch);
    m_bgm->set_gain(gain);
    m_bgm->set_position(position);
    m_bgm->set_velocity(velocity);
    m_bgm->set_reference_distance(reference_distance);
    m_bgm->set_max_distance(max_distance);
    m_bgm->set_rolloff(rolloff);

    if(playing)
      m_bgm->play();
#endif
  }

  Sound_Buffer & Sound::get_Hello_World_Buffer() const {
    if(!m_hello_world_buffer)
      m_hello_world_buffer = new Sound_Buffer();

    return *m_hello_world_buffer;
  }

#ifdef DISABLE_AL
  Sound_Source & Sound::get_BGM_Source() const {
    static Sound_Source bgm_source;

    if(!m_bgm) {
      m_bgm = new Sound_Buffer();

      bgm_source.set_buffer(*m_bgm);
      bgm_source.set_priority(ZENI_DEFAULT_MUSIC_PRIORITY);
    }

    return bgm_source;
  }
#else
  void Sound::init_BGM_Sound_Stream_AL() const {
    if(!m_bgm)
      m_bgm = new Sound_Stream_AL("sfx/104469__dkmedic__world");
  }
#endif

  Sound & get_Sound() {
    return Sound::get();
  }

}

#include <Zeni/Undefine.h>
