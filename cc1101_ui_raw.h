// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#ifndef _CC1101_UI_RAW_H_
#define _CC1101_UI_RAW_H_

#include <memory>

#include <ui.h>

class CC1101;

std::shared_ptr<UI> make_cc1101_raw_ui(std::shared_ptr<CC1101> &cc1101);

#endif