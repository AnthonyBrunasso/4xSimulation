#include "notification.h"

#include <algorithm>
#include <sstream>

#include "city.h"
#include "game_types.h"
#include "player.h"
#include "unit.h"


namespace notification {
  NotificationVector get_player_notifications(uint32_t player_id) {
    Player* player = player::get_player(player_id);
    if (!player) {
      return NotificationVector();
    }
    NotificationVector events;
    player::for_each_player_city(player_id, [&events] (City& c) {
      city::do_notifications(c.m_id, events);
    });
    player::for_each_player_unit(player_id, [&events] (Unit& u) {
      if (u.m_path.empty() && u.m_action_points > 0) {
        Notification n;
        n.m_event_type = fbs::NOTIFICATION_TYPE::UNIT_IDLE;
        n.m_id = u.m_id;
        events.push_back(n);
      }
    });
    if (player->m_research == SCIENCE_TYPE::UNKNOWN) {
      Notification n;
      n.m_id = 0;
      n.m_event_type = fbs::NOTIFICATION_TYPE::SCIENCE_IDLE;
    }

    return std::move(events);
  }
 
  std::string to_string(Notification& n) {
    std::stringstream ss;
    if (n.m_event_type == fbs::NOTIFICATION_TYPE::UNKNOWN) {
      ss << "Unknown notification." ;
    }
    else if (n.m_event_type == fbs::NOTIFICATION_TYPE::CITY_STARVING) {
      ss << "City (" << n.m_id << ") is starving." ;
    }
    else if (n.m_event_type == fbs::NOTIFICATION_TYPE::CITY_DEFENSE) {
      ss << "City (" << n.m_id << ") is vulnerable to attack by unit " << n.m_other_id ;
    }
    else if (n.m_event_type == fbs::NOTIFICATION_TYPE::CITY_HARVEST) {
      ss << "City (" << n.m_id << ") has an idle_worker." ;
    }
    else if (n.m_event_type == fbs::NOTIFICATION_TYPE::CITY_PRODUCTION) {
      ss << "City (" << n.m_id << ") construction has idle_queue." ;
    }
    else if (n.m_event_type == fbs::NOTIFICATION_TYPE::CITY_SPECIALIZE) {
      ss << "City (" << n.m_id << ") has knowledge of the local terrain, specialize <cityId> <terrainType>." ;
    }
    else if (n.m_event_type == fbs::NOTIFICATION_TYPE::UNIT_IDLE) {
      ss << "Unit " << n.m_id << " is idle." ;
    }
    else if (n.m_event_type == fbs::NOTIFICATION_TYPE::SCIENCE_IDLE) {
      ss << "You require a new research task." ;
    }
    else {
      ss << "Unhandled notification" << std::endl;
    }

    return ss.str();
  }
}


