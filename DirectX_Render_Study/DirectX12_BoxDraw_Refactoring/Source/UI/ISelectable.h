#pragma once

class ISelectable
{
public:
    virtual ~ISelectable() = default;

    // フォーカスが当たった時
    virtual void OnSelect() = 0;
    
    // フォーカスが外れた時
    virtual void OnDeselect() = 0;
    
    // 決定ボタンが押された時
    virtual void OnSubmit() = 0;
};
