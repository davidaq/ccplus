mapProperty({
    name: 'gaussian',
    order: -5,
    map:['Effects/Gaussian Blur/Blurriness','Effects/Gaussian Blur/Blur Dimensions'],
    set:function(blurriness,dimensions) {
        return [
            blurriness, dimensions
        ];
    },
    error:[0.1, 0.001]
});
