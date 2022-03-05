/*
 * OpenKneeboard
 *
 * Copyright (C) 2022 Fred Emmott <fred@fredemmott.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */
#pragma once

#include <stop_token>
#include <memory>
#include <mutex>
#include <vector>

#include <OpenKneeboard/Events.h>

namespace OpenKneeboard {

struct GameInstance;

class GameInjector final {
 public:
  bool Run(std::stop_token);

  Event<GameInstance> evGameChanged;
  void SetGameInstances(const std::vector<GameInstance>&);

 private:
  std::vector<GameInstance> mGames;
  std::mutex mGamesMutex;
};

}