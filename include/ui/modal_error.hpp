/* MODAL LOGIC
Simple modal component to display error information.
Contains one exit button to close the modal
*/

#pragma once

#include <ftxui/component/component.hpp>

using namespace ftxui;

class BonsaiModalError {
public:
    static Component error(const std::string& subtitle, std::function<void()> exit);
};