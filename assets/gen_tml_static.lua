function makeMain(template, medias, userinfo) 
    local user_medias = prepareUserMedias(userinfo);
    for i = 1, #user_medias do 
        local media = user_medias[i]
        if media.type == "text" then
        elseif media.type == "image" or media.type == "video" then
            local compName = "@" .. tostring(i)
            local d = template.compositions[compName].duration
            media.comp.duration = d
            media.comp.layers[1].duration = d
            media.comp.layers[1].last = d
            template.compositions[compName] = media.comp
        end 
    end 
end
