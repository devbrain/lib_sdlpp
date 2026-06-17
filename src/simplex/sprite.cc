#include <simplex/sprite.hh>
#include <algorithm>

namespace simplex {
    void animated_sprite::add_animation(std::string name, animation_sequence seq) {
        m_animations[std::move(name)] = std::move(seq);
    }

    void animated_sprite::play(const std::string& name) {
        if (m_current_anim == name) {
            return;
        }

        auto it = m_animations.find(name);
        if (it != m_animations.end()) {
            m_current_anim = name;
            m_current_frame_idx = 0;
            m_timer = 0.0f;
            if (!it->second.frames.empty()) {
                m_active_frame = it->second.frames[0];
            }
        }
    }

    void animated_sprite::stop() noexcept {
        m_current_anim.clear();
        m_current_frame_idx = 0;
        m_timer = 0.0f;
    }

    void animated_sprite::update(float dt) {
        if (m_current_anim.empty()) {
            return;
        }

        auto it = m_animations.find(m_current_anim);
        if (it == m_animations.end() || it->second.frames.empty()) {
            return;
        }

        const auto& seq = it->second;
        if (seq.fps <= 0.0f) {
            return;
        }

        m_timer += dt;
        float frame_duration = 1.0f / seq.fps;

        while (m_timer >= frame_duration) {
            m_timer -= frame_duration;
            if (m_current_frame_idx + 1 < seq.frames.size()) {
                m_current_frame_idx++;
            } else if (seq.loop) {
                m_current_frame_idx = 0;
            } else {
                m_timer = 0.0f;
                break;
            }
        }

        m_active_frame = seq.frames[m_current_frame_idx];
    }
} // namespace simplex
