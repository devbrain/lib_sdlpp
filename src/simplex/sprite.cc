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
        // 1. Process animation sequence updates if playing
        if (!m_current_anim.empty()) {
            auto it = m_animations.find(m_current_anim);
            if (it != m_animations.end() && !it->second.frames.empty()) {
                const auto& seq = it->second;
                if (seq.fps > 0.0f) {
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
            }
        }

        // 2. Process movement scripting queue updates
        while (!m_movement_queue.empty() && dt > 0.0f) {
            auto& step = m_movement_queue.front();

            if (m_move_timer == 0.0f) {
                m_move_start_pos = position;
            }

            if (step.duration <= 0.0f) {
                position = m_move_start_pos + step.target_offset;
                m_movement_queue.pop_front();
                m_move_timer = 0.0f;
            } else {
                float remaining_time = step.duration - m_move_timer;
                if (dt >= remaining_time) {
                    position = m_move_start_pos + step.target_offset;
                    dt -= remaining_time;
                    m_movement_queue.pop_front();
                    m_move_timer = 0.0f;
                } else {
                    m_move_timer += dt;
                    float t = m_move_timer / step.duration;
                    float eased_t = step.easing_fn ? step.easing_fn(t) : t;
                    position = m_move_start_pos + step.target_offset * eased_t;
                    dt = 0.0f;
                }
            }
        }
    }

    void animated_sprite::queue_movement(movement_step step) {
        m_movement_queue.push_back(std::move(step));
    }

    void animated_sprite::clear_movements() noexcept {
        m_movement_queue.clear();
        m_move_timer = 0.0f;
    }
} // namespace simplex
