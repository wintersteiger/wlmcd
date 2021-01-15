// Copyright (c) Christoph M. Wintersteiger
// Licensed under the MIT License.

#include <cmath>
#include <cstring>

#include "ui.h"

std::mutex UI::mtx;
WINDOW *UI::mainw = NULL, *UI::logp = NULL, *UI::logboxw = NULL, *UI::cmdw = NULL, *UI::statusp = NULL;
unsigned  UI::screen_height = 0, UI::screen_width = 0;
uint64_t UI::reset_cnt = 0;
uint64_t UI::indicator_value = 0, UI::max_indicator_value = 0;

UI::UI() :
  logp_scrollback(0),
  active_field_index((size_t)-1)
{
  Reset();
}

UI::~UI()
{
  // for (auto f: fields)
  //  delete(f);
  for (size_t i=0; i < fields.size(); i++) {
    delete(fields[i]);
    fields[i] = NULL;
  }
}

void UI::Start()
{
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);

  start_color();
  // init_pair(name, fore, back)
  init_pair(HIGH_PAIR, COLOR_BLACK, COLOR_GREEN);
  init_pair(LOW_PAIR, COLOR_BLACK, COLOR_RED);
  init_pair(ENABLED_PAIR, COLOR_GREEN, COLOR_BLACK);
  init_pair(DISABLED_PAIR, COLOR_RED, COLOR_BLACK);
  init_pair(HIGHLIGHT_PAIR, COLOR_YELLOW, COLOR_BLACK);
  init_pair(STALE_PAIR, COLOR_BLACK, COLOR_BLACK);
}

void UI::Reset()
{
  mtx.lock();
  wclear(stdscr);
  wrefresh(stdscr);
  getmaxyx(stdscr, screen_height, screen_width);

  logp_b = 999;
  if (!logp) {
    logp_w = screen_width-2;
    logp = newpad(logp_b, logp_w);
    scrollok(logp, TRUE);
    wmove(logp, logp_b-1, 0);
  }
  else if (screen_width - 2 > logp_w) {
    logp_w = screen_width-2;
    wresize(logp, logp_b, logp_w);
  }

  if (!logboxw)
    logboxw = newwin(0, 0, 0, 0);

  if (!cmdw) {
    cmdw = newwin(0, 0, 0, 0);
    scrollok(cmdw, FALSE);
    keypad(cmdw, TRUE);
    nodelay(UI::cmdw, TRUE);
  }

  int logboxw_h = std::max(screen_height/4, 3U);
  int logboxw_w = std::max(screen_width, 3U);
  int logboxw_x = 0;
  int logboxw_y = screen_height - logboxw_h - 1;
  wresize(logboxw, logboxw_h, logboxw_w);
  mvwin(logboxw, logboxw_y, logboxw_x);
  box(logboxw, 0, 0);

  logp_h = logboxw_h - 2;
  logp_w = logboxw_w - 2;
  logp_y = logboxw_y + 1;
  logp_x = logboxw_x + 1;
  mvwin(logp, logp_y, logp_x);
  prefresh(logp, logp_b-logp_h, 0, logp_y, logp_x, logp_y + logp_h, logp_x + logp_w);

  if (!statusp)
    statusp = newpad(screen_height, screen_width * 2);
  else
    wclear(statusp);
  wresize(statusp, screen_height - logboxw_h - 1, 2*screen_width);

  wresize(cmdw, 1, screen_width);
  mvwin(cmdw, screen_height-1, 0);

  for (auto w : {logboxw, cmdw}) {
    wrefresh(w);
    // wclear(w);
  }

  active_field_index = (size_t)-1;
  for (auto f: fields)
    if (f) f->Active(false);

  Layout();
  mtx.unlock();

  Update(true);
  ResetCmd();
  reset_cnt++;
}

void UI::ScrollUp()
{
  logp_scrollback += logp_h;
  if (logp_scrollback > logp_b - logp_h)
    logp_scrollback = logp_b - logp_h;
}

void UI::ScrollDown()
{
  logp_scrollback -= logp_h;
  if (logp_scrollback < 0)
    logp_scrollback = 0;
}

void UI::Update(bool full)
{
  mtx.lock();
  curs_set(0);
  for (auto f: fields)
    if (f) f->Update(full);
  int pheight, pwidth;
  getmaxyx(statusp, pheight, pwidth);
  prefresh(statusp, 0, 0, 0, 0, pheight-1, screen_width-1);

#if 1
  static char indicator_buf[32];
  size_t indicator_width = std::floor(std::log10(max_indicator_value)) + 1;
  snprintf(indicator_buf, sizeof(indicator_buf), "[%0llu]", indicator_value);
  mvwprintw(logboxw, logp_h+1, screen_width - (2 + strlen(indicator_buf)), indicator_buf);
  wrefresh(logboxw);
#endif

  // prefresh(w, (y, x) in pad, (y1, x1, y2, x2) on screen);
  int logp_r = logp_b-logp_scrollback-logp_h;
  prefresh(logp, logp_r, 0, logp_y, logp_x, logp_y + logp_h - 1, logp_x + logp_w - 1);
  wrefresh(cmdw);
  curs_set(1);
  mtx.unlock();
}

int UI::End() {
  mtx.lock();
  for (WINDOW **w: { &logp, &logboxw, &cmdw, &statusp, &mainw }) {
    if (*w) delwin(*w);
    *w = NULL;
  }
  int r = endwin();
  mtx.unlock();
  return r;
}

void UI::ResetCmd() {
  mtx.lock();
  wclear(cmdw);
  wmove(cmdw, 0, 0);
  wrefresh(cmdw);
  mtx.unlock();
}

static const char *current_minutes() {
  static char time_buf[256];
  time_t t;
  time(&t);
  struct tm * lt = localtime(&t);
  strftime(time_buf, sizeof(time_buf), "%H:%M:%S", lt);
  return time_buf;
}

int UI::Log(const char *format, ...)
{
  int r = 0;
  if (logp) {
    mtx.lock();
    va_list argp;
    va_start(argp, format);
    wprintw(logp, "\n%s> ", current_minutes());
    r = vw_printw(logp, format, argp);
    va_end(argp);
    wrefresh(logp);
    mtx.unlock();
  }
  return r;
}

void UI::Error(const char *format, ...)
{
  wclear(cmdw);
  wattron(cmdw, COLOR_PAIR(LOW_PAIR));
  va_list argp;
  va_start(argp, format);
  static const char *prefix = "ERROR: ";
  mvwprintw(cmdw, 0, 0, prefix);
  vw_printw(cmdw, format, argp);
  va_end(argp);
  wattroff(cmdw, COLOR_PAIR(LOW_PAIR));
}

void UI::Info(const char *format, ...)
{
  wclear(cmdw);
  wattron(cmdw, COLOR_PAIR(HIGHLIGHT_PAIR));
  va_list argp;
  va_start(argp, format);
  static const char *prefix = "> ";
  mvwprintw(cmdw, 0, 0, prefix);
  vw_printw(cmdw, format, argp);
  va_end(argp);
  wattroff(cmdw, COLOR_PAIR(HIGHLIGHT_PAIR));
}

void UI::Add(FieldBase *field)
{
  fields.push_back(field);
}

int UI::GetKey()
{
  return wgetch(cmdw);
}

void UI::Layout()
{
  size_t h, w, widest = 0;
  size_t r = 0, c = 1;
  getmaxyx(statusp, h, w);

  int last_r = -1;
  int last_c = -1;
  for (size_t i=0; i < fields.size(); i++) {
    FieldBase *f = fields[i];
    int this_c = c;

    if (f->Row() == last_r) {
      this_c += f->Col() - last_c;
    }
    else {
      last_c = f->Col();
      last_r = f->Row();
      r++;
    }

    if (r >= h) {
      r = 1;
      c += widest + 1;
      widest = 0;
      this_c = c;
    }

    f->Move(r, this_c);

    size_t w = f->Width();
    if (w > widest)
      widest = w;
  }
}

void UI::First()
{
  if (active_field_index < fields.size()) {
    FieldBase *f = fields[active_field_index];
    f->Active(false);
    f->Update(true);
  }

  if (active_field_index == (size_t)-1 ||
      active_field_index < fields.size()) {
    for (size_t next = 0; next < fields.size(); next++) {
      if (fields[next]->Activateable()) {
        active_field_index = next;
        break;
      }
    }
  }

  if (active_field_index < fields.size()) {
    FieldBase *f = fields[active_field_index];
    f->Active(true);
    f->Update(true);
  }

  Update(false);
}

void UI::Previous() // key up
{
  if (active_field_index < fields.size()) {
    FieldBase *f = fields[active_field_index];
    f->Active(false);
    f->Update(true);
  }

  if (active_field_index != (size_t)-1) {
    if (active_field_index > fields.size())
      active_field_index = fields.size();
    for (size_t prev = active_field_index - 1; prev != (size_t)-2; prev--) {
      if (prev == (size_t)-1 || fields[prev]->Activateable()) {
        active_field_index = prev;
        break;
      }
    }
  }

  if (active_field_index < fields.size()) {
    FieldBase *f = fields[active_field_index];
    f->Active(true);
    f->Update(true);
  }

  Update(false);
}

void UI::Next() // key down
{
  if (active_field_index < fields.size()) {
    FieldBase *f = fields[active_field_index];
    f->Active(false);
    f->Update(true);
  }

  if (active_field_index == (size_t)-1 ||
      active_field_index < fields.size() - 1) {
    for (size_t next = active_field_index + 1; next < fields.size(); next++) {
      if (fields[next]->Activateable()) {
        active_field_index = next;
        break;
      }
    }
  }

  if (active_field_index < fields.size()) {
    FieldBase *f = fields[active_field_index];
    f->Active(true);
    f->Update(true);
  }

  Update(false);
}

void UI::Last()
{
  if (active_field_index < fields.size()) {
    FieldBase *f = fields[active_field_index];
    f->Active(false);
    f->Update(true);
  }

  if (active_field_index == (size_t)-1 ||
      active_field_index < fields.size()) {
    for (size_t prev = fields.size() - 1; prev != (size_t)-2; prev--) {
      if (prev == (size_t)-1 || fields[prev]->Activateable()) {
        active_field_index = prev;
        break;
      }
    }
  }

  if (active_field_index < fields.size()) {
    FieldBase *f = fields[active_field_index];
    f->Active(true);
    f->Update(true);
  }

  Update(false);
}

void UI::Edit()
{
  char tmp[256];

  wclear(cmdw);

  if (active_field_index >= fields.size())
    return;

  FieldBase *f = fields[active_field_index];

  if (f->ReadOnly()) {
    Error("field is read-only");
    return;
  }

  mtx.lock();
  echo();
  mvwprintw(cmdw, 0, 0, "new value: ");
  nodelay(cmdw, FALSE);
  wgetnstr(cmdw, tmp, sizeof(tmp));
  nodelay(cmdw, TRUE);
  noecho();
  mtx.unlock();

  if (active_field_index < fields.size() &&
      !fields[active_field_index]->ReadOnly())
  {
    FieldBase *f = fields[active_field_index];
    f->Set(tmp);
    // UI::Log("Set %s to %s", f->Key().c_str(), tmp);
  }
  else
    UI::Log("Input: %s", tmp);

  ResetCmd();
}

std::string UI::GetCommand()
{
  char tmp[256];

  wclear(cmdw);

  mtx.lock();
  echo();
  curs_set(2);
  mvwprintw(cmdw, 0, 0, ":");
  nodelay(cmdw, FALSE);
  wgetnstr(cmdw, tmp, sizeof(tmp));
  nodelay(cmdw, TRUE);
  curs_set(1);
  noecho();
  mtx.unlock();

  ResetCmd();

  return tmp;
}

void UI::Describe()
{
  int y1 = screen_height/4;
  int x1 = screen_width/4;
  int y2 = 3 * screen_height/4;
  int x2 = 3 * screen_width/4;
  WINDOW* dwin = newwin(screen_height/2, screen_width/2, y1, x1);
  scrollok(dwin, FALSE);
  nodelay(dwin, FALSE);
  wtimeout(dwin, 250);
  box(dwin, 0, 0);
  wrefresh(dwin);
  curs_set(0);

  std::string text = "(no description)";
  if (active_field_index < fields.size())
    text = fields[active_field_index]->Describe();

  WINDOW *descp = newpad(y2-y1-2, x2-x1-2);
  mvwprintw(descp, 0, 0, text.c_str());
  // prefresh(w, (y, x) in pad, (y1, x1, y2, x2) on screen);
  prefresh(descp, 0, 0, y1+1, x1+1, y2-2, x2-2);

  char k = 0;
  do {
    k = wgetch(dwin);
  }
  while (k != '\n' && k != KEY_ENTER && k != 27 && k != 'q');

  delwin(descp);
  delwin(dwin);

  Update(true);
}

void UI::Bump()
{
  if (active_field_index >= fields.size())
    return;

  FieldBase *f = fields[active_field_index];
  f->Bump();
  f->Update(true);
}
