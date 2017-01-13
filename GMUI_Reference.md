# GMLib UI Toolkit Controls Reference
Below is a list of UI control classed implemented in GMLib and available to applications. Also applications should provide their own implementation of derived `ui::control` or any of the higher level control classes, such as `ui::box`.

## ui::manager
The `ui::manager` class is a core of the UI toolkit and a root of the ui controls created with it. The class is an implementation of `screen::component` interface but at the same time it is a singleton. The `ui::manager` is initialized with GMLib and accessable via `ui::manager::instance()` interface. Some of more often used methods in `ui::manager` were proxied to `ui` namespace.
* `void ui::pop_front(control *)`
* `void ui::push_back(control *)`
* `control * ui::get_hovered_control()`
* `void ui::set_pointer(theme::pointer::pointer_type)`
* `theme::pointer::pointer_type ui::get_pointer_type()`
* `void ui::destroy(control *)`
* `template<class T> T * ui::build(data const *)`

Each class in this reference can be created either programatically `auto btn = new ui::button(...);` or declaratively in `.ui.json` file. The files are loaded with `control * ui::manager::build(data const *)` method, or it's template version in `ui` namespace.

## ui::color
Color class is a basic data class used by other controls to specify configurable graphics settings.
Colors are represented as array of 4 ints or as a constant string of known name.
`[r, g, b, a]` - array of 4 ints red, green, blue and alpha value. 0 - 255.
### Color constants
The value of color field in JSON can be a string of constant value below.
 * `"red"`
 * `"green"`
 * `"blue"`
 * `"dark_red"`
 * `"dark_green"`
 * `"dark_blue"`
 * `"black"`
 * `"white"`
 * `"dark_gray"`
 * `"gray"`
 * `"light_gray"`

### Theme constants
Theme provides a `theme::skin` implementation for a control to render itself, it contains colors and fonts specified in the `.theme.json` file loaded from `config`. The following values are recognized and used from current control skin's settings. 
* `"back"` - background color
* `"idle"` - foreground color, normal variant
* `"highlight"` - forground color, extra variant (_i.e. hovered, selected, etc_)

## ui::control
The core, root class used to define a control managed by `ui::manager`. Providers core functionality of positioning, rendering and updating. This class is not directly available in JSON, but each control has `ui::control` as a parent. 
The `ui::control` uses the following JSON properties making them mandatory for all controls.
* __id__        - unique identifier. If missing in json then a new id is generated and assigned.
* __rect__      - absolute position of the control (optional, conflicts with [position])
* __position__  - relative position of the control (optional, conflicts with [rect])
                `"center"` - placed at the center of the parent container
               `[x, y]` - array of 2 ints with relative x and y
* __size__      - `[width, height]` - array of 2 ints.
                required by __position__ and as hint in auto-sizeing in boxes.

## ui::box
Generic container class with automatic positioning (stacking) of child controls.
##### JSON name: `"box"`
##### JSON Properties:
* __type__      - the type of a box.
                `"hbox"` - horizontal packing
                `"vbox"` - vertical packing
* __style__    - style of the children positioning
                `"pack_start"` - begin at the left or top
                `"pack_end"`   - begin at the right or bottom
                `"fill"`       - equaly space and resize to fill
                `ui::box::box_style` - int value of the enum
* __scroll__    - optional, default: hidden. the type of a scrollbar used for the box
                `"right"`  - right edge of the box  
                `"bottom"` - botton ed       ge of the box
* __scrollbar_size__ - int width or height of the scrollbar rect, default is 16px.
* __margin__         - int space between box contents and box frame in pixels
                   

## ui::panel
Container class derived from `ui::box` with panel background and frame rendering.
##### JSON name: `"panel"`
##### JSON Properties:
* __panel_style__   - the frame style selector in the current theme
                    `"dialog"` - regular frame for normal windows (default)
                    `"toolbox"` - smaller frame for tool windows
                    `"group"` - tiny frame for control groups
* __background__    - _color_ of a background area

## ui::label
A control implementation for displaing static text and or image.
##### JSON name: `"label"`
##### JSON Properties:
JSON properties:
* __text__              - label text string
* __icon__              - icon resource
* __font__              - theme font selection: `"normal"`, `"bold"`, `"ital"`
* __font_idle_color__   - _color_ of the idle label's text
* __font_hover_color__  - _color_ of the hovered label's text
* __v_align__           - label contents vertical alignment: `"top"` (default), `"botton"` and `"middle"`
* __h_align__           - label contents horizontal alignment: `"left"` (default), `"right"` and `"center"`.
* __icon_pos__          - icon position relative to the text: `"left"` (default) or `"right"`.
* __icon_gap__          - int space between text and icon in pixels
* __margin__           - space between label contents and label frame in pixels
                        `[top, left, bottom, right]` - array of 4 ints
                        `int` value to initialize all 4 directions.

## ui::text_input
A control for user input based on `ui::label`.
##### JSON name: `"input"`
##### JSON Properties:
* __validation__ - filter user input: `"whitespace"`, `"alpha"`, `"numbers"`, `"alphanum"` and `"everything"` (default)
* __readonly__   - ignore user input
* __frame__      - eanble frame rendering

## ui::btn, ui::sbtn and ui::lbtn
GMLib provides 3 types of inteface `ui::button` to applications. `ui:btn` and `ui:sbtn` are 2 themes button styles different in sizes. While 'ui:lbtn' is a minimal style button with fixed `ui::shape` as it's visual representation.
##### JSON name: `"btn"`, `"sbtn"`, `"lbtn"`
##### JSON Properties: 
_Derived only._

## ui::combo
A combo box control provides a fixed selection by extending `ui:text_input` to display a panel of avaialble items to select.
The panel is implemented with `ui:box` and provides scrolling.
JSON Name: combo
##### JSON name: `"combo"`
##### JSON Properties:
* __area_maxlen__     - max lenght of the items area in pixels
* __area_color__      - _color_ of the area background
* __items__           - array of dictionaries with _label_ control data.