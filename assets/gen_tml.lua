local currentTime, overlap = 0, 0
local preferredDuration, bgm_volume = 6.0, {}
local last_scene = nil
local template, scenes, medias, ret_layers = nil, nil, nil, nil
local random_candicate_medias = {}
local scene_idx = 0
local scenes

local function appendScene(name, scene) 
    local layer = {
        uri= "composition://" .. name,
        time= currentTime,
        duration= scene.duration - overlap,
        start= overlap,
        last= scene.duration - overlap,
        properties= { transform= {}}
    }
    if layer.duration < 0 then return end 
    layer.properties.transform["0"] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0}
    table.insert(ret_layers, layer)

    overlap = scene.overlap
    currentTime = currentTime + layer.duration

    if preferredDuration == 6.0 then
        preferredDuration = 3.6;
    elseif preferredDuration == 3.6 then
        preferredDuration = 6.0
    end 
end 

local function appendTextScene(media) 
    -- Assume only one text scene
    local text_scene = nil
    for i=1, #scenes do 
        if scenes[i].type == "text" and scenes[i].num_text == #media.text then 
            text_scene = scenes[i] 
            break
        end 
    end 
    if not text_scene then return nil end
    local last = false
    if not last_scene and scene_idx == #medias - 1 then
        last = true
    end 
    text_scene.fill(scene_idx, media, last)
    appendScene("$" .. scene_idx, text_scene)
    print(media.text[1], "=>", text_scene.name)
    scene_idx = scene_idx + 1
end 

local last_volume = true
local function appendNormalScene(media) 
    local sceneEval = function(scene) 
        if scene.type == "text" or scene.name == "#LAST" then 
            return 0x7fffffff
        end 
        local diff = 0
        if media.type == "image" then 
            diff = (scene.duration > 7.0) and math.floor(math.abs(media.comp.duration - scene.duration)) or 0  
        elseif media.type == "video" then
            diff = math.floor(math.abs(media.comp.duration - scene.duration))
        end 
        return diff * 100 + math.abs(math.floor(preferredDuration - scene.duration)) * 10 + scene.used
    end 
    bubbleSort(scenes, sceneEval)
    -- Assume there is at least one solution
    local matched_rand_comps = {}
    local scene = scenes[1]
    for i = 1, scene.num_rand_ele do
        local tmp, try = math.random(#random_candicate_medias), 1
        while tmp == scene_idx and try < 3 do 
            tmp = math.random(#random_candicate_medias) 
            try = try + 1
        end 
        table.insert(matched_rand_comps, random_candicate_medias[tmp])
    end 
    if media.has_volume then
        bgm_volume[tostring(currentTime)] = last_volume and 1.0 or 0.1;
        bgm_volume[tostring(currentTime + 0.5)] = 0.1;
        bgm_volume[tostring(currentTime + scene.duration - overlap - 0.5)] = 0.1;
        bgm_volume[tostring(currentTime + scene.duration - overlap)] = 1.0;
    end 
    last_volume = not media.has_volume
    local last = false
    if scene_idx == #medias - 1 and not last_scene then
        last = true
    end 
    print(media.name, "=>", scene.name)
    scene.fill(scene_idx, media, matched_rand_comps, last)
    appendScene("$" .. scene_idx, scene)
    scene_idx = scene_idx + 1
    scene.used = scene.used + 1
end 

local callback_map = {
    image= appendNormalScene,
    video= appendNormalScene,
    text= appendTextScene
}

local function initLayer(uri, duration) 
    local l = {
        start= 0, last= duration,
        time= 0, duration= duration,
        uri= uri,
        properties= {transform= {}}
    }
    return l
end 

function makeMain(_template, _medias, userinfo) 
    local idx = 1
    template = _template
    local comps = template.compositions
    scenes = prepareScenes(template);
    medias = _medias
    for i = 1, #medias do 
        local media = medias[i]
        if media.type ~= "text" then
            template.compositions[media.name] = media.comp
        end 
    end 
    for i=1, #medias do
        if medias[i].type == "image" then
            table.insert(random_candicate_medias, medias[i].name)
        end 
    end 
    if #random_candicate_medias == 0 then 
        for i=1, #medias do
            if medias[i].type ~= "text" then
                table.insert(random_candicate_medias, medias[i].name)
            end 
        end 
    end 
    template.compositions[template.main] = {
        resolution= {
            width= WIDTH,
            height= HEIGHT
        },
        layers= {}
    }
    local main_comp = template.compositions[template.main]
    ret_layers = main_comp.layers

    -------------------------
    -- Title
    -------------------------
    -- local start_comp, text_comp = comps['#TITLE'], comps['#T']
    -- if text_comp and start_comp and userinfo.videoTitle and userinfo.videoTitle:len() > 0 then 
    --     text_comp.layers[1]['text-properties']['text']["0"] = userinfo.videoTitle
    --     local l = initLayer("composition://#TITLE", start_comp.duration)
    --     l.properties.transform["0"] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0}
    --     table.insert(ret_layers, l)
    -- end

    -------------------------
    -- Scenes
    ------------------------
    last_scene = nil
    for i = 1, #scenes do if scenes[i].name == "#LAST" then last_scene = scenes[i] end end
    if last_scene and last_scene.num_rand_ele > #random_candicate_medias then
        last_scene = nil
    end 
    while idx <= #medias do
        local media = medias[idx]
        callback_map[media.type](media)
        idx = idx + 1
    end 
    -- #LAST
    if last_scene then
        last_scene.fill(scene_idx, media, random_candicate_medias, true)
        appendScene("$" .. scene_idx, last_scene)
        scene_idx = scene_idx + 1
    end 
    -- #END
    appendScene("#END", comps["#END"])
    local l = initLayer("composition://#END", comps["#END"].duration)

    main_comp.duration = currentTime

    -------------------------
    -- Background music
    ------------------------
    bgm_volume["0"] = 0.0
    if not bgm_volume["0.5"] then
        bgm_volume["0.5"] = 1.0
    end 
    bgm_volume[tostring(main_comp.duration - 1.5)] = 1.0
    bgm_volume[tostring(main_comp.duration)] = 0.0
    template.bgm_volume  = bgm_volume
    if userinfo.musicURL and userinfo.musicURL ~= "" then 
        local l = initLayer("xfile://" .. userinfo.musicURL, main_comp.duration)
        l.properties.volume = {}
        for tm, val in pairs(bgm_volume) do 
            l.properties.volume[tm] = {val};
        end
        table.insert(ret_layers, l)
    end 
end
