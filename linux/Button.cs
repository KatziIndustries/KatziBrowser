public class Button
{
    public float x;
    public float px;
    public float y;
    public float w;
    public float h;
    public bool Hovered;
    public bool Clicked;

    public Button(float x, float y, float w, float h, bool rightaligned)
    {
        this.x = x;
        this.y = y;
        this.w = w;
        this.h = h;

        Hovered = false;
        Clicked = false;
    }

    public void Update(float mx, float my, float ww)
    {
        px = this.x + ww;

        Hovered =
            mx >= px &&
            mx < px + w &&
            my >= y &&
            my < y + h;
    }
}
