mapProperty({
    name: 'hsl',
    order: -5,
    map:[
        ['Effects','Hue/Saturation','Master Hue'],
        ['Effects','Hue/Saturation','Master Saturation'],
        ['Effects','Hue/Saturation','Master Lightness'],
    ],
    alias:{
        'Hue/Saturation':'Color Balance (HLS)',
        'Master Hue':'Hue',
        'Master Saturation':'Saturation',
        'Master Lightness':'Lightness'
    },
    set:function(h,s,l) {
        return [
            h / 2, 1 + s / 100, 1 + l / 100
        ];
    }
});
