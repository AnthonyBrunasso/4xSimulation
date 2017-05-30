#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace fbs {
  enum class NOTIFICATION_TYPE : uint32_t; 
}

struct Notification
{
  fbs::NOTIFICATION_TYPE m_event_type;
  uint32_t m_id;
  uint32_t m_other_id;
};

typedef std::vector<Notification> NotificationVector;

namespace notification
{
  NotificationVector get_player_notifications(uint32_t player_id);
  std::string to_string(Notification&);
};
