using Color = System.Drawing.Color;
using SmashFramework;

public static class Draw
{
    public static void Circle(Renderer renderer, float px, float py, float dx, float dy, float i, Color color)
    {
        for (int j = 0; j < i; j++)
        {
            float w = dx * MathF.Sin(
                j * ((MathF.PI / 2) / (i + 1))
            );

            float h = dy * MathF.Cos(
                j * ((MathF.PI / 2) / (i + 1))
            );

            renderer.RenderFilledRectangle(
                new(
                    px - w,
                    py - h,
                    2 * w,
                    2 * h
                ),
                color
            );
        }
    }
}

