#ifndef EDITOR_WINDOW_H
#define EDITOR_WINDOW_H

class EditorWindow {
public:
    virtual ~EditorWindow() {}

    void Show() {
        visible = true;
    }
    void Update() {
        if(!visible) return;
        Draw();
    }

    virtual void Draw() = 0;
protected:
    bool visible = false;
};

#endif
