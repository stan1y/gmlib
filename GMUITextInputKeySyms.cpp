#include "GMUITextInput.h"

namespace ui {

#define SDL_SHIFT_PRESSED(kbdstate) (kbdstate[SDL_SCANCODE_LSHIFT] || kbdstate[SDL_SCANCODE_RSHIFT])

std::string text_input::translate_sym(const uint8_t * kbdstate, const SDL_Keycode & sym)
{
  std::string letter;
  switch(sym) {
  case SDLK_a:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "A" : "a";
    break;
  case SDLK_b:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "B" : "b";
    break;
  case SDLK_c:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "C" : "c";
    break;
  case SDLK_d:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "D" : "d";
    break;
  case SDLK_e:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "E" : "e";
    break;
  case SDLK_f:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "F" : "f";
    break;
  case SDLK_g:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "G" : "g";
    break;
  case SDLK_h:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "H" : "h";
    break;
  case SDLK_i:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "I" : "i";
    break;
  case SDLK_j:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "J" : "j";
    break;
  case SDLK_k:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "K" : "k";
    break;
  case SDLK_l:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "L" : "l";
    break;
  case SDLK_m:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "M" : "m";
    break;
  case SDLK_n:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "N" : "n";
    break;
  case SDLK_o:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "O" : "o";
    break;
  case SDLK_p:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "p" : "p";
    break;
  case SDLK_q:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "Q" : "q";
    break;
  case SDLK_r:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "R" : "r";
    break;
  case SDLK_s:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "S" : "s";
    break;
  case SDLK_t:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "T" : "t";
    break;
  case SDLK_u:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "U" : "u";
    break;
  case SDLK_v:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "V" : "v";
    break;
  case SDLK_w:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "W" : "w";
    break;
  case SDLK_x:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "X" : "x";
    break;
  case SDLK_y:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "Y" : "y";
    break;
  case SDLK_z:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "Z" : "z";
    break;

  case SDLK_0:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? ")" : "0";
    break;
  case SDLK_1:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "!" : "1";
    break;
  case SDLK_2:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "@" : "2";
    break;
  case SDLK_3:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "#" : "3";
    break;
  case SDLK_4:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "$" : "4";
    break;
  case SDLK_5:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "%" : "5";
    break;
  case SDLK_6:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "^" : "6";
    break;
  case SDLK_7:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "&" : "7";
    break;
  case SDLK_8:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "*" : "8";
    break;
  case SDLK_9:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "(" : "9";
    break;

  case SDLK_SPACE:
    letter = " ";
    break;
  case SDLK_EQUALS:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "=" : "+";
    break;
  case SDLK_MINUS:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "-" : "_";
    break;
  case SDLK_SLASH:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "/" : "?";
    break;
  case SDLK_BACKSLASH:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "\\" : "|";
    break;

  case SDLK_COMMA:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "," : "<";
    break;
  case SDLK_STOP:
    letter = SDL_SHIFT_PRESSED(kbdstate) ? "." : ">";
    break;

  };
  return letter;
}

};