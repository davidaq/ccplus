mapProperty({
    name: 'opacity',
    order: -5,
    map:'Opacity',
    set:function(opac) {
        return [opac / 100];
    },
});
