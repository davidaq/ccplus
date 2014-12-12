mapProperty({
    name: '4color',
    order: 10,
    map:[
        'Effects/4-Color Gradient/Point 1',
        'Effects/4-Color Gradient/Color 1',
        'Effects/4-Color Gradient/Point 2',
        'Effects/4-Color Gradient/Color 2',
        'Effects/4-Color Gradient/Point 3',
        'Effects/4-Color Gradient/Color 3',
        'Effects/4-Color Gradient/Point 4',
        'Effects/4-Color Gradient/Color 4',
        'Effects/4-Color Gradient/Blend',
        'Effects/4-Color Gradient/Opacity',
        'Effects/4-Color Gradient/Blending Mode',
    ],
    set:function(pos1, color1, pos2, color2, pos3, color3, pos4, color4, blend, opac, mode) {
        try {
            mode = [0, -1, 0, 1, 2, 3][mode];
        } catch(E) {
            mode = 0;
        }
        return [
            pos1[0], pos1[1],
            color1[2] * 255, color1[1] * 255, color1[0] * 255,
            pos2[0], pos2[1],
            color2[2] * 255, color2[1] * 255, color2[0] * 255,
            pos3[0], pos3[1],
            color3[2] * 255, color3[1] * 255, color3[0] * 255,
            pos4[0], pos4[1],
            color4[2] * 255, color4[1] * 255, color4[0] * 255,
            blend,
            opac / 100,
            mode
        ];
    }
});
