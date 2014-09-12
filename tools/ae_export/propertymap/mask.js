mapProperty({
    name: 'mask',
    order: 5,
    map:['Masks/1/Mask Feather','Masks/1/maskShape'],
    set:function(feather, maskShape) {
        var ret = [];
        ret.push(feather[0]);
        ret.push(feather[1]);
        for(var k = 0; k < maskShape.vertices.length; k++) {
            var pnt = maskShape.vertices[k];
            ret.push(pnt[1]);
            ret.push(pnt[0]);
            var nk = (k + 1) % maskShape.vertices.length;
            var p0 = maskShape.vertices[k];
            var p1 = maskShape.outTangents[k];
            var p2 = maskShape.inTangents[nk];
            var p3 = maskShape.vertices[nk];
            p1[0] += p0[0];
            p1[1] += p0[1];

            p2[0] += p3[0];
            p2[1] += p3[1];
            var cuts = 7;
            var step = 1.0 / cuts;
            for(var i = 1; i < cuts; i++) {
                var t = step * i;
                var rt = 1 - t;
                var px = rt * rt * rt * p0[0] + 3 * t * rt * rt * p1[0] + 3 * t * t * rt * p2[0] + t * t * t* p3[0];
                var py = rt * rt * rt * p0[1] + 3 * t * rt * rt * p1[1] + 3 * t * t * rt * p2[1] + t * t * t* p3[1];
                ret.push(py);
                ret.push(px);
            }
        }
        return ret;
    },
});
