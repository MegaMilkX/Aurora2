#ifndef EDITOR_WINDOW_H
#define EDITOR_WINDOW_H

class EditorWindow {
public:
    virtual ~EditorWindow() {}

    void Show() {
        visible = true;
    }
    bool Update() {
        if(!visible) return false;
        return Draw();
    }

    virtual bool Draw() = 0;
protected:
    bool visible = false;
};

#endif
