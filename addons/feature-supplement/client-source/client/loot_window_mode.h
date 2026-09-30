#pragma once

// Shared policy for the solo selector and automatic group override.
// The view implementation must exist before this policy is connected to the UI.
namespace nms_loot {
enum class SoloWindow { Classic, Modern };

class WindowMode {
public:
    void SetPreference(SoloWindow value) { preference_ = value; }
    SoloWindow Preference() const { return preference_; }
    SoloWindow Effective(bool grouped, bool pending_group_rolls) const {
        // Do not strand an unresolved group roll when its player leaves a group.
        return grouped || pending_group_rolls ? SoloWindow::Modern : preference_;
    }

private:
    SoloWindow preference_ = SoloWindow::Modern;
};
}
