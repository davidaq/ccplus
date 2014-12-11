mapProperty({
    name: 'ramp',
    order: 10,
    map:[
        'Effects/Ramp/Start of Ramp',
        'Effects/Ramp/Start Color',
        'Effects/Ramp/End of Ramp',
        'Effects/Ramp/End Color',
        'Effects/Ramp/Ramp Shape',
        'Effects/Ramp/Blend With Original'
    ],
    alias:{
        'Ramp':'Gradient Ramp'
    },
    set:function(startPos, startColor, endPos, endColor, shape, blend) {
        return [
            shape == 1 ? -1 : 1,
            startPos[0], startPos[1],
            startColor[2] * 255, startColor[1] * 255, startColor[0] * 255,
            endPos[0], endPos[1],
            endColor[2] * 255, endColor[1] * 255, endColor[0] * 255,
            blend / 100
        ];
    }
});
