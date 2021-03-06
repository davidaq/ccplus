package.path = package.path .. ";" .. DKJSON_PATH;
local json = require("dkjson");

WIDTH = 640
HEIGHT = 640

----------------------------------------------
-- Helpers
----------------------------------------------
function log(tb) 
    print(json.encode(tb, {indent= true}));
end

function simpleLastIndexOf(s, pattern) 
    local i = s:match(".*"..pattern.."()")
    if i == nil then return nil else return i - 1 end
end

function deepcopy(orig)
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

function spairs(t)
    local keys = {}
    for k in pairs(t) do keys[#keys+1] = k end
    table.sort(keys)

    -- return the iterator function
    local i = 0
    return function()
        i = i + 1
        if keys[i] then
            return keys[i], t[keys[i]]
        end
    end
end

--[==[
-- return [{name= ~, duration= ~, num_ele= ~, paths= ~}]
--]==]
function getScenes(template) 
    local ret = {};
    local comps = template.compositions;
    function countUserElements(layers, pname, paths, used) 
        local cnt = 0
        local cnt_random = 0
        local got_child = false
        for i = 1, #layers do 
            local uri = layers[i].uri
            if string.sub(uri, 1, 14) == "composition://" then
                if (string.sub(uri, 15, 15) == "@") then
                    got_child = true
                    local id = 1
                    if #uri > 15 then
                        id = tonumber(string.sub(uri, 16));
                    end
                    if not used[id] and id ~= 0 then
                        cnt = cnt + 1
                        used[id] = true
                    end
                    if id == 0 then
                        cnt_random = cnt_random + 1
                    end
                else 
                    local cname = string.sub(uri, 15)
                    if comps[cname] then 
                        local tmp_cnt, tmp_rand_cnt, tmp_got_child = countUserElements(comps[cname].layers, cname, paths, used)
                        cnt = cnt + tmp_cnt
                        cnt_random = cnt_random + tmp_rand_cnt
                        if tmp_got_child then 
                            got_child = true
                            if paths[pname] == nil then
                                paths[pname] = {}
                            end 
                            table.insert(paths[pname], cname)
                        end 
                    end
                end 
            end 
        end
        return cnt, cnt_random, got_child
    end
    for cname, comp in spairs(comps) do 
        if string.sub(cname, 1, 1) == "#" and cname ~= "#+1" and string.upper(cname) ~= "#COVER" and string.upper(cname) ~= "#TEXT" then
            local layers = comp.layers;
            local num_ele 
            local paths = {}
            num_ele, num_rand_ele = countUserElements(layers, cname, paths, {})
            if num_ele > 0 or num_rand_ele > 0 then
                table.insert(ret, {
                    name= cname,
                    duration= comp.duration,
                    num_ele= num_ele,
                    num_rand_ele= num_rand_ele,
                    used= 0,
                    paths= paths
                });
            end 
        end 
    end 

    --log(ret);
    return ret;
end

--[==[
-- return [{name= ~, comp= ~}]
--]==]
function generateResourcesComp(userinfo) 
    local medias = userinfo.medias
    local ret = {}
    local idx = 0
    for k, md in pairs(medias) do 
        if md.type == "text" then 
            table.insert(ret, {
                type= md.type,
                text= md.text,
            })
        else
            local comp = {}
            local name = "@" .. idx
            if md.type == "image" then
                comp.duration = 3.0
            elseif md.type == "video" then 
                comp.duration = 5.0
            end
            if md.duration then 
                comp.duration = tonumber(md.duration)
            end
            local start = 0
            if md.start then
                start = tonumber(md.start)
            end 

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
                if type(md.volume) == "string" then 
                    md.volume = tonumber(md.volume) 
                end
                l.properties.volume = {}
                l.properties.volume["0"] = {md.volume}
            end

            -- call c func
            local has_volume = false;
            if md.type == "video" and md.volume and md.volume ~= "0" then
                local path = md.filename
                if string.sub(path, 1, 1) ~= "/" then 
                    path = USER_JSON_DIR .. path
                end
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
    --log(ret)
    return ret
end

--[==[
-- Core matching algorithm -- currently stupid
-- return [scene_name, [comp_name1, comp_name2, ...], paths]
--]==]
function fit(comps, scenes) 
    local idx = 1
    local ret = {}
    local image_comps = {}
    for i = 1, #comps do 
        if comps[i].type ~= "video" then
            table.insert(image_comps, comps[i])
        end
    end
    local preferredDuration = 6.0
    function sortScenes(s, eval)
        for i=1, #s do 
            for j=i+1, #s do
                if eval(s[i]) > eval(s[j]) then
                    s[i], s[j] = s[j], s[i]
                end
            end 
        end
    end
    -- Try simple scene
    while idx <= #comps do 
        local comp_cnt = #comps - idx + 1
        sortScenes(scenes, function(a) 
            if comp_cnt < a.num_ele then return 0x7fffffff end
            if a.name == "#LAST" then -- Ignore #last now
                return 0x7fffffff
            end
            local diff = 0
            for i = idx, idx + a.num_ele - 1 do 
                if comps[i]["type"] == "video" then
                    if a.duration > comps[i].comp.duration then
                        return 0x7fffffff
                    end
                    diff = diff + (comps[i].comp.duration - a.duration) 
                elseif comps[i]["type"] == "image" then
                    if a.duration > 7.0 then 
                        diff = diff + a.duration - comps[i].comp.duration
                    end
                end
            end
            return a.used * 1 + diff * 100 + math.abs(preferredDuration - a.duration) * 10
        end)

        local scene = scenes[1]
        if scene.num_ele > comp_cnt then
            break
        end
        local tmp = {
            name= scene.name,
            matched_comps= {},
            matched_rand_comps= {},
            scene_path= scene.paths,
            mute_bgm= false
        }
        if scene.num_ele == 1 and comps[idx].has_volume then
            tmp.mute_bgm = true
        end
        for i = idx, idx + scene.num_ele - 1 do 
            table.insert(tmp.matched_comps, comps[i].name)
        end 
        local rand = {}
        for i = 1, scene.num_rand_ele do 
            --local tmp_i = math.random(#comps)
            --if not rand[tmp_i] then
            --    table.insert(tmp.matched_rand_comps, comps[tmp_i].name)
            --    rand[tmp_i] = true
            --end
            if #image_comps > 0 then
                table.insert(tmp.matched_rand_comps, comps[math.random(#image_comps)].name)
            else 
                table.insert(tmp.matched_rand_comps, comps[math.random(#comps)].name)
            end
        end 
        idx = idx + scene.num_ele 
        scene.used = scene.used + 1
        table.insert(ret, tmp)
        if preferredDuration == 6.0 then
            preferredDuration = 3.6;
        elseif preferredDuration == 3.6 then
            preferredDuration = 6.0
        end 
    end
    -- Try #LAST scene use random
    local last_scene
    for i = 1, #scenes do
        if scenes[i].name == "#LAST" then
            last_scene = scenes[i]
        end
    end
    if last_scene and last_scene.num_rand_ele <= #image_comps and last_scene.num_ele <= #image_comps then
        local tmp = {}
        for i = 1, last_scene.num_ele do
            table.insert(tmp, image_comps[i].name)
        end
        local rand_tmp = {}
        for i = 1, last_scene.num_rand_ele do
            table.insert(rand_tmp, image_comps[i].name)
        end
        table.insert(ret, {
            name= last_scene.name, 
            matched_comps= tmp, 
            matched_rand_comps= rand_tmp,
            scene_path= last_scene.paths,
            mute_bgm= false
        })
    end
    log(ret)
    return ret
end

-- Fill template
function fillTML(fitted, template, userinfo, aux_template)
    local candidates = {}
    local comps = template.compositions
    for i = 1, #fitted do 
        local fit = fitted[i]
        local cname = fit.name
        local paths = fit.scene_path
        local overlap = 0
        local idx = 1
        function cloneScene(name, newname) 
            local comp = deepcopy(comps[name])
            local layers = comp.layers
            for l = 1, #layers do
                layer = layers[l]
                if layer.uri:sub(15, 15) == "@" then
                    if layer.uri:sub(16) == "0" then 
                        layer.uri = "composition://" .. fit.matched_rand_comps[idx]
                        layer.properties.volume = {};
                        layer.properties.volume["0"] = {0};
                        idx = idx + 1
                    else
                        local id = 1
                        if #layer.uri >= 16 then
                            id = tonumber(layer.uri:sub(16))
                        end 
                        layer.uri = "composition://" .. fit.matched_comps[id]
                    end
                elseif layer.uri == "composition://#+1" then
                    if i ~= #fitted then 
                        layer.uri = "composition://$" .. (i + 1)
                    else
                        layer.uri = "composition://#END"
                    end
                    -- FIXME: should calculate relative time
                    -- Currently, generally #+1 will be on the top-most comp
                    overlap = comp.duration - layer.time;
                end 
            end 
            local children = paths[name]
            if type(children) == "table" then
                local cloned = {}
                for ch = 1, #children do
                    local child = children[ch]
                    local tmp = child .. "$" .. i .. "ch" .. ch
                    cloneScene(child, tmp)
                    for l = 1, #layers do
                        if layers[l].uri:sub(15) == child and not cloned[l] then
                            layers[l].uri = "composition://" .. tmp
                            cloned[l] = true
                            break
                        end 
                    end 
                end 
            end

            if name:sub(1, 1) == "#" then
                comps["$" .. i] = comp
            else
                comps[newname] = comp
            end
            return comp
        end 
        local comp = cloneScene(cname)
        table.insert(candidates, {"$" .. i, comp.duration, overlap, fit.mute_bgm})
    end 
    
    -- Copy compositions from aux to comps
    local comps_aux = aux_template.compositions
    for cname, comp in pairs(comps_aux) do 
        if cname ~= "MAIN" then
            template.compositions[cname] = comp
        end
    end

    --
    -- Generate MAIN composition
    --
    local main_name = template.main
    comps[main_name] = {
        resolution= {
            width= WIDTH,
            height= HEIGHT
        },
        layers= {}
    }
    local main_comp = comps[main_name]
    local ret_layers = main_comp.layers
    local bgm_volume = {}
    local currentTime = 0
    local overlap = 0
    function appendScene(name, duration) 
        local layer = {
            uri= "composition://" .. name,
            time= currentTime,
            duration= duration - overlap,
            start= overlap,
            last= duration - overlap,
            properties= {
                transform= {
                }
            }
        }
        if layer.duration < 0 then
            return 
        end 
        layer.properties.transform["0"] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0}
        currentTime = currentTime + layer.duration
        table.insert(ret_layers, layer)
    end
    
    -- Fill text and append TITLE
    local start_comp = comps['#TITLE']
    local text_comp = comps['#T']
    if text_comp and start_comp and userinfo.videoTitle and userinfo.videoTitle:len() > 0 then 
        text_comp.layers[1]['text-properties']['text']["0"] = userinfo.videoTitle
        local l = {
            start= 0,
            time= 0,
            duration= start_comp.duration,
            last= start_comp.duration,
            uri= "composition://#TITLE",
            properties= {
                transform= {}
            }
        }
        l.properties.transform["0"] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0}
        table.insert(ret_layers, l)
    end

    -- Append scenes
    for i = 1, #candidates do 
        if candidates[i][4] then -- If mute BGM
            if i == 1 or not candidates[i - 1][4] then 
                bgm_volume[tostring(currentTime)] = 1.0;
            else 
                bgm_volume[tostring(currentTime)] = 0.1;
            end
            bgm_volume[tostring(currentTime + 0.5)] = 0.1;
            bgm_volume[tostring(currentTime + candidates[i][2] - overlap - 0.5)] = 0.1;
            bgm_volume[tostring(currentTime + candidates[i][2] - overlap)] = 1.0;
        end 
        appendScene(candidates[i][1], candidates[i][2])
        overlap = candidates[i][3]
    end
    --log(bgm_volume)

    -- Append end scenes
    appendScene("#END", comps["#END"].duration)

    -- Append background music
    main_comp.duration = currentTime
    bgm_volume["0"] = 0.0
    if not bgm_volume["0.5"] then
        bgm_volume["0.5"] = 1.0
    end
    bgm_volume[tostring(main_comp.duration - 1.5)] = 1;
    bgm_volume[tostring(main_comp.duration)] = 0;
    template.bgm_volume = bgm_volume
    if userinfo.musicURL and userinfo.musicURL ~= "" then
        local music = {
            start= 0,
            time=0, 
            duration= main_comp.duration,
            last= main_comp.duration,
            properties= {
                volume= {}
            },
            uri= "xfile://" .. userinfo.musicURL
        }

        for tm, val in pairs(bgm_volume) do 
            music.properties.volume[tm] = {val};
        end

        table.insert(ret_layers, music)
    end 
end 

function urisToGlobal(template, globalPath) 
    if globalPath == "" then return end
    if globalPath:sub(globalPath:len()) ~= "/" then
        globalPath = globalPath .. "/"
    end
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

function rescaleTemplate(template, scale) 
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

    function resize(props, resizer)
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

local template = json.decode(TPL_JSON, 1, nil);
local userinfo = json.decode(USER_JSON, 1, nil);
local aux_template = json.decode(TPL_AUX_JSON, 1, nil)
if USER_JSON_DIR:sub(USER_JSON_DIR:len()) ~= "/" then
    USER_JSON_DIR = USER_JSON_DIR .. "/"
end

-- Deal with URI in template
local templateDir = userinfo.templateURL
local slash_pos = simpleLastIndexOf(templateDir, "%/") 
if slash_pos then 
    templateDir = templateDir:sub(1, slash_pos)
else
    templateDir = ""
end
urisToGlobal(template, templateDir)

-- TODO Deal with URI in aux tpl
if ASSETS_PATH and ASSETS_PATH ~= "" then
    if ASSETS_PATH:sub(#ASSETS_PATH) ~= "/" then
        ASSETS_PATH = ASSETS_PATH .. "/"
    end
    urisToGlobal(aux_template, ASSETS_PATH .. "aux_tpl")
end 

local scenes = getScenes(template);
local comps = generateResourcesComp(userinfo);
for i = 1, #comps do 
    template.compositions[comps[i].name] = comps[i].comp;
end

fillTML(fit(comps, scenes), template, userinfo, aux_template)

if JSON_BEAUTIFY == nil then
    JSON_BEAUTIFY = true
end
if HALF_SIZE then
    rescaleTemplate(template, 0.6)
end
RESULT = json.encode(template, {
    indent= JSON_BEAUTIFY
})
