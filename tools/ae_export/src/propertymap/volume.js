mapProperty({
    name: 'volume',
    order: -5,
    map:'Audio Levels',
    set:function(lvl) {
        if(typeof(lvl) == 'object') {
            var v = 0;
            var n = 0;
            for(k in lvl) {
                if(typeof(lvl[k]) == 'number') {
                    v += lvl[k];
                    n++;
                }
            }
            if(n > 0)
                v /= n;
            lvl = v;
        }
        return [(lvl + 48) / 48];
    },
})
