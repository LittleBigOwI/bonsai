/* MODAL CONFIRM
- Simple modal component to confirm an action.
- Contains two buttons (action = confirm & exit = cancel) that will each execute their respective function on click
*/

#pragma once

#include <ftxui/component/component.hpp>

using namespace ftxui;

class BonsaiModalConfirm {
public:
    static Component confirm(std::function<void()> action, std::function<void()> exit);
};