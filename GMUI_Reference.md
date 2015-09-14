== GMLib UI Toolkit Controls Reference ==

-- ui::control --
Inherits: none
JSON name: none
Description: A core control class. 
             Base class for all controls and UI manager.
JSON Properties (mandatory):
* id       - unique identifier. If missing in json then 
             a new [id] is generated and assigned.
* rect     - absolute position of the control (optional, conflicts with [position])
* position - relative position of the control (optional, conflicts with [rect])
             * center - placed at the center of the parent container
             * [x, y] - array of 2 ints with relative x and y
* size     - [width, height] array of 2 ints.
             required by [position] and as hint in auto-sizeing in boxes.

-- ui::box, ui:panel --
Inherits: ui::control
JSON Name: box, panel
Description: Generic containers for groups of controls with scrolling.
             The panel class can render a frame of several types.
JSON Properties:
* type           - the type of a box.
                   * hbox - horizontal packing
                   * vbox - vertical packing
* style          - style of the children positioning
                   * pack_start - begin at the left or top
                   * pack_end   - begin at the right or bottom
                   * fill       - equaly space and resize to fill
                   * int value of the [ui::box::box_style] enum
* scroll         - optional, default: hidden. the type of a scrollbar used for the box
                   * right  - right edge of the box  
                   * bottom - botton ed       ge of the box
* scrollbar_size - width or height of the scrollbar rect
                   default is 16px
* margin         - space between box contents and box frame in pixels
                   * int value in pixels for all directions
                   * [top, left, bottom, right] array of 4 ints

-- ui::label --
Inherits: ui::control
JSON Name: label 
Description: A label control displays a text and optional icon.
JSON properties:
* text      - label text string
* icon      - icon resource
* v_align   - label contents vertical alignment
              * top (default)
              * botton
              * middle
* h_align   - label contents horizontal alignment
              * left (default)
              * right
              * center
* icon_pos   - icon position relative to the text
               * left (default)
               * right
* icon_gap   - space between text and icon in pixels
* margin     - space between label contents and label frame in pixels
               * int value in pixels for all directions
               * [top, left, bottom, right] array of 4 ints

-- ui::text_input --
Inherits: ui::label
JSON name: input
Description: A text input control with validation.
JSON Properties:
* validation - filter user input. (Masks combinations)
             * whitespace
             * alpha
             * numbers
             * alphanum
             * everything (default)

-- ui::btn, ui::sbtn --
Inherits: label
JSON Name: btn, sbtn
Description: 2 push button classes. Button ans Small button.
             Use different theme frames and sizes.
No JSON Properties