mapProperty({
    name: 'transform',
    order: 0,
    map:['Position','Anchor Point','Scale', 'Orientation','X Rotation','Y Rotation','Z Rotation'],
    set:function(pos, anchor, scale, orientation, rotateX, rotateY, rotateZ) {
        if(!rotateX)
            rotateX = 0;
        if(orientation && orientation[0])
            rotateX += orientation[0];
        if(!rotateY)
            rotateY = 0;
        if(orientation && orientation[1])
            rotateY += orientation[1];
        if(!rotateZ)
            rotateZ = 0;
        if(orientation && orientation[2])
            rotateZ += orientation[2];
        return [
            pos[0], pos[1], pos[2],
            anchor[0], anchor[1], anchor[2],
            scale[0] * 0.01, scale[1] * 0.01, scale[2] * 0.01,
            rotateX, rotateY, rotateZ
        ];
    },
    error:[0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01]
});
