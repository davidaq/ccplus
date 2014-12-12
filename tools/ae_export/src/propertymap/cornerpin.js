mapProperty({
    name: '4corner',
    order: 1,
    map:[
        'Effects/Corner Pin/Upper Left',
        'Effects/Corner Pin/Upper Right',
        'Effects/Corner Pin/Lower Left',
        'Effects/Corner Pin/Lower Right',
    ],
    set:function(ul, ur, ll, lr) {
        return [ul[0], ul[1], ur[0], ur[1], ll[0], ll[1], lr[0], lr[1]];
    }
});
