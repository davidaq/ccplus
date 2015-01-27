package.path = package.path .. ";" .. DKJSON_PATH;
local json = require("dkjson");

WIDTH = 640
HEIGHT = 640

----------------------------------------------
-- Helpers
----------------------------------------------
local function log(tb) 
    print(json.encode(tb, {indent= true}));
end

local function simpleLastIndexOf(s, pattern) 
    local i = s:match(".*"..pattern.."()")
    if i == nil then return nil else return i - 1 end
end

local function deepcopy(orig)
    local orig_type = type(orig)
    local copy
    if orig_type == 'table' then
        copy = {}
        for orig_key, orig_value in next, orig, nil do
            copy[deepcopy(orig_key)] = deepcopy(orig_value)
        end
        setmetatable(copy, deepcopy(getmetatable(orig)))
    else -- number, string, boolean, etc
        copy = orig
    end
    return copy
end

local function spairs(t)
    local keys = {}
    for k in pairs(t) do keys[#keys+1] = k end
    table.sort(keys)
    local i = 0
    return function()
        i = i + 1
        if keys[i] then
            return keys[i], t[keys[i]]
        end
    end
end

local function bubbleSort(s, eval)
    for i=1, #s do 
        for j=i+1, #s do
            if eval(s[i]) >= eval(s[j]) then
                s[i], s[j] = s[j], s[i]
            end
        end 
    end
end

-- Merge t2 into t1
local function mergeTable(t1, t2) 
    for k,v in pairs(t2) do t1[k] = v end
end 


------------------------------------------
-- Prepare scene
------------------------------------------
-- Get search_path, num_ele, num_rand_ele, overlap
local function scanScene(comps, name) 
    local comp = comps[name]
    local layers = comp.layers
    local num_rand_ele, search_path, overlap, has_ele = 0, {}, 0, false
    for i = 1, #layers do 
        local layer = layers[i]
        local uri = layer.uri
        if uri == "composition://@" then
            has_ele = true
        elseif uri == "composition://@0" then 
            has_ele = true
            num_rand_ele = num_rand_ele + 1
        elseif (layer["text-properties"] and layer["text-properties"]["text"]["0"] == "#TITLE#") or 
            (layer["text-properties"] and layer["text-properties"]["text"]["0"] == "#SUBTITLE#") then
            has_ele = true
        elseif uri == "composition://#+1" then 
            -- FIXME: Currently #+1 was assumed to be in first level layer
            overlap = comp.duration - layer.time
        elseif string.sub(uri, 1, 14) == "composition://" then 
            local tmp_search_path, tmp_num_rand_ele, tmp_overlap, tmp_has_ele = scanScene(comps, uri:sub(15))
            if tmp_has_ele then
                mergeTable(search_path, tmp_search_path)
                has_ele = has_ele or tmp_has_ele
                num_rand_ele = num_rand_ele + tmp_num_rand_ele
                if not search_path[name] then 
                    search_path[name] = {}
                end 
                table.insert(search_path[name], uri)
            end 
        end 
    end 
    return search_path, num_rand_ele, overlap, has_ele
end 

local function cloneAndFill(scene, newname, comps, callback)
    local queue = {{scene.name, newname}}
    -- BFS
    while #queue > 0 do
        local tmp = queue[#queue]
        local name, newname = tmp[1], tmp[2]
        table.remove(queue)

        local comp = deepcopy(comps[name])
        local layers = comp.layers
        for l = 1, #layers do 
            local layer = layers[l]
            callback(layer)
        end 
        local children = scene.search_path[name]
        if children then
            local cloned = {}
            for ch = 1, #children do
                local child = children[ch]:sub(15)
                local tmp = child .. newname .. "ch" .. ch
                table.insert(queue, {child, tmp})

                for l = 1, #layers do 
                    if layers[l].uri:sub(15) == child and not cloned[l] then
                        layers[l].uri = "composition://" .. tmp
                        cloned[l] = true
                        break
                    end 
                end 
            end 
        end 
        comps[newname] = comp
    end 
end 

local function prepare(comps, name)
    local comp = comps[name]
    local ret = {
        name= name,
        duration= comp.duration,
        used= 0
    }
    local search_path, num_rand_ele, overlap, has_ele = scanScene(comps, name)
    ret.overlap = overlap
    ret.search_path = search_path
    if name == "#TEXT" or name == "#TEXT_SINGLE" then
        ret.type = "text"
        ret.num_text = (name == "#TEXT") and 2 or 1
        ret.fill = function(idx, media, last)
            cloneAndFill(ret, "$" .. idx, comps, function(layer) 
                if layer["text-properties"] and layer["text-properties"]["text"]["0"] == "#TITLE#" then
                    layer.uri = "text://" .. media.text[1]
                    layer["text-properties"]["text"]["0"] = media.text[1]
                elseif layer["text-properties"] and layer["text-properties"]["text"]["0"] == "#SUBTITLE#" then
                    layer.uri = "text://" .. media.text[2]
                    layer["text-properties"]["text"]["0"] = media.text[2]
                elseif layer.uri == "composition://#+1" then
                    if last then
                        layer.uri = "composition://#END" 
                    else
                        layer.uri = "composition://$" .. (idx + 1)
                    end
                end 
            end)
        end 
    else 
        if not has_ele then return nil end 
        ret.num_rand_ele = num_rand_ele
        ret.type = "normal"
        ret.fill = function(idx, media, matched_rand_comps, last)
            local idx_rand = 1
            cloneAndFill(ret, "$" .. idx, comps, function(layer) 
                if layer.uri == "composition://@" then
                    layer.uri = "composition://" .. media.name
                elseif layer.uri == "composition://@0" then
                    layer.uri = "composition://" .. matched_rand_comps[idx_rand]
                    layer.properties.volume = {}
                    layer.properties.volume["0"] = {0}
                    idx_rand = idx_rand + 1
                elseif layer.uri == "composition://#+1" then
                    if last then
                        layer.uri = "composition://#END" 
                    else
                        layer.uri = "composition://$" .. (idx + 1)
                    end
                end 
            end)
        end
    end
    return ret
end 

local function prepareScenes(template) 
    local ret = {}
    local comps = template.compositions
    local exception = {}
    exception["#COVER"] = true
    exception["#TITLE"] = true
    exception["#T"] = true
    exception["#END"] = true
    
    for name, comp in pairs(comps) do 
        if name:sub(1, 1) == "#" and name ~= "#+1" and not exception[string.upper(name)] then
            local scene = prepare(comps, name)
            if scene then table.insert(ret, scene) end
        end
    end
    return ret
end 

------------------------------------------
-- Prepare user data
------------------------------------------
local function prepareUserMedias(userinfo) 
    local medias, idx = userinfo.medias, 0
    local ret = {}
    for k, md in pairs(medias) do
        if md.type == "text" then 
            table.insert(ret, {
                type= md.type,
                text= md.text
            })
        else
            local comp = {}
            local name = "@" .. idx
            if md.duration then comp.duration = tonumber(md.duration)
            elseif md.type == "image" then comp.duration = 3.0
            elseif md.type == "video" then comp.duration = 5.0
            end
            local start = 0
            if md.start then start = tonumber(md.start) end 

            comp.resolution = {
                width= WIDTH,
                height= HEIGHT
            }

            if type(md.x) == "string" then md.x = tonumber(md.x) end
            if type(md.y) == "string" then md.y = tonumber(md.y) end
            if type(md.w) == "string" then md.w = tonumber(md.w) end
            if type(md.h) == "string" then md.h = tonumber(md.h) end
            local l = {
                uri= "xfile://" .. md.filename,
                time= 0,
                duration= comp.duration,
                start= start,
                last= comp.duration, 
                properties= {
                    transform= {}
                }
            }
            l.properties.transform["0"] = {
                WIDTH / 2.0, HEIGHT / 2.0, 0, 
                md.x + md.w / 2.0, md.y + md.h / 2.0, 0, 
                WIDTH / md.w, HEIGHT / md.h, 1,
                0, 0, 0
            }
            comp.layers = {l}

            if md.volume then 
                if type(md.volume) == "string" then md.volume = tonumber(md.volume) end
                l.properties.volume = {}
                l.properties.volume["0"] = {md.volume}
            end

            -- call c func
            local has_volume = false
            if md.type == "video" and md.volume and md.volume ~= 0 then
                local path = md.filename
                if string.sub(path, 1, 1) ~= "/" then path = USER_JSON_DIR .. path end
                has_volume = hasVolume(path, start, comp.duration)
            end 

            table.insert(ret, {
                name= name,
                type= md.type,
                comp= comp,
                has_volume= has_volume
            })
            idx = idx + 1
        end
    end 
    return ret
end

------------------------------------------
-- Generate main composition
------------------------------------------

local function _generateMainComp() 
    local currentTime, overlap = 0, 0
    local preferredDuration, bgm_volume = 6.0, {}
    local last_scene = nil
    local template, scenes, medias, ret_layers = nil, nil, nil, nil
    local random_candicate_medias = {}
    local scene_idx = 0

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

    return function(_template, _scenes, _medias, userinfo) 
        local idx = 1
        template = _template
        local comps = template.compositions
        scenes = _scenes
        medias = _medias
        for i=1, #medias do
            if medias[i].type == "image" then
                table.insert(random_candicate_medias, medias[i].name)
            end 
        end 
        if #random_candicate_medias == 0 then 
            for i=1, #medias do
                if medias[i].type ~= "text" then
                    table.insert(random_candicate_medias, medias[i])
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
end

local generateMainComp = _generateMainComp()

-----------------------------------------
-- Template Helpers 
-----------------------------------------
local function urisToGlobal(template, globalPath) 
    if globalPath == "" then return end
    if globalPath:sub(#globalPath) ~= "/" then globalPath = globalPath .. "/" end
    local comps = template.compositions
    for name, comp in pairs(comps) do
        for i = 1, #comp.layers do
            local uri = comp.layers[i].uri
            if uri:sub(1, 7) == "file://" and uri:sub(8, 8) ~= "/" then 
                comp.layers[i].uri = "file://" .. globalPath .. uri:sub(8)
            end
        end
    end
end

local function rescaleTemplate(template, scale) 
    local resizers = {
        transform= function(len) 
            local ret = {}
            for i = 1, len, 12 do 
                for j = 0, 5 do
                    table.insert(ret, i + j)
                end 
            end
            return ret
        end,
        mask= function(len)
            local ret = {}
            for i = 1, len do
                table.insert(ret, i)
            end
            return ret
        end,
        ramp= function (len)
            return {1, 2}
        end
    }
    resizers["4corner"] = function() 
        return {1, 2, 3, 4, 5, 6, 7, 8}
    end
    resizers["4color"] = function() 
        return {1, 2, 6, 7, 11, 12, 16, 17}
    end

    local function resize(props, resizer)
        if not props then return end
        for time, prop in pairs(props) do
            local keys = resizer(#prop)
            for i = 1, #keys do 
                prop[keys[i]] = prop[keys[i]] * scale
            end
        end
    end

    for name, comp in pairs(template.compositions) do
        comp.resolution.width = comp.resolution.width * scale
        comp.resolution.height = comp.resolution.height * scale
        for i = 1, #comp.layers do 
            local layer = comp.layers[i]
            for pname, resizer in pairs(resizers) do
                if pname == "mask" then
                    if layer.uri:sub(1, 14) == "composition://" then
                        resize(layer.properties[pname], resizer)
                    end
                else
                    resize(layer.properties[pname], resizer)
                end
            end
            local trans = layer.properties.transform
            if trans and layer.uri:sub(1, 14) ~= "composition://" then
                for time, prop in pairs(trans) do
                    local newprop = {0, 0, 0, 0, 0, 0, scale, scale, 1, 0, 0, 0}
                    for i = 1, #prop do
                        table.insert(newprop, prop[i])
                    end
                    layer.properties.transform[time] = newprop
                end
            end
        end
    end
end

-----------------------------------------
-- Main Logic 
-----------------------------------------

-- A random number
math.randomseed(1421121622)

local template = json.decode(TPL_JSON, 1, nil);
local userinfo = json.decode(USER_JSON, 1, nil);
local aux_template = json.decode(TPL_AUX_JSON, 1, nil)
if USER_JSON_DIR:sub(USER_JSON_DIR:len()) ~= "/" then USER_JSON_DIR = USER_JSON_DIR .. "/" end

-- URI in template
local templateDir = userinfo.templateURL
local slash_pos = simpleLastIndexOf(templateDir, "%/") 
if slash_pos then 
    templateDir = templateDir:sub(1, slash_pos)
else
    templateDir = ""
end
urisToGlobal(template, templateDir)
-- URI in aux tpl
if ASSETS_PATH and ASSETS_PATH ~= "" then
    if ASSETS_PATH:sub(#ASSETS_PATH) ~= "/" then
        ASSETS_PATH = ASSETS_PATH .. "/"
    end
    urisToGlobal(aux_template, ASSETS_PATH .. "aux_tpl")
end 

-- Copy aux to template
local aux_comps = aux_template.compositions
for k, v in pairs(aux_comps) do 
    if not template.compositions[k] then
        template.compositions[k] = v
    end
end
local scenes = prepareScenes(template);
local user_medias = prepareUserMedias(userinfo);
for i = 1, #user_medias do 
    local media = user_medias[i]
    if media.type ~= "text" then
        template.compositions[media.name] = media.comp
    end 
end 
generateMainComp(template, scenes, user_medias, userinfo)

-- MAIN

if JSON_BEAUTIFY == nil then JSON_BEAUTIFY = true end
if HALF_SIZE then rescaleTemplate(template, 0.6) end
RESULT = json.encode(template, {indent= JSON_BEAUTIFY})
