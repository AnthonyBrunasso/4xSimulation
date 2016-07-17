#pragma once

#include <cstdint>
#include <vector>
#include <string>

enum class NOTIFICATION_TYPE;

struct Notification
{
  NOTIFICATION_TYPE m_event_type;
  uint32_t m_id;
};

typedef std::vector<Notification> NotificationVector;

namespace notification
{
  NotificationVector get_player_notifications(uint32_t player_id);
  std::string to_string(Notification&);
};
