// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _EVOHOME_UI_H_
#define _EVOHOME_UI_H_

#include <ui.h>
#include <evohome.h>

class EvohomeUI : public UI {
protected:
  time_t last_update;
  const Evohome::State &state;

public:
  EvohomeUI(const Evohome::State &state);
  virtual ~EvohomeUI();

  virtual void Update(bool full);

  virtual std::string Name() const { return "Evohome"; }

protected:
  void Build();
};

#endif
