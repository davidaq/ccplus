mapProperty({
    name: 'grayscale',
    order: -5,
    map:[
        'Effects/Black & White/Reds',
        'Effects/Black & White/Yellows',
        'Effects/Black & White/Greens',
        'Effects/Black & White/Cyans',
        'Effects/Black & White/Blues',
        'Effects/Black & White/Magentas',
        'Effects/Black & White/Tint:',
        'Effects/Black & White/Tint Color',
    ],
    set:function(r,y,g,c,b,m,haveTint,tintColor) {
        var ret = [r,y,g,c,b,m,0,0];
        if(haveTint > 0) {
            var hs = getHueSat(tintColor);
            ret[6] = hs[0];
            ret[7] = hs[1];
        }
        return ret;
    }
});
