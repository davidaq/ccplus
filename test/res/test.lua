local json = require("assets/dkjson");
print "JSON TEST";

local tmp = {
    hello = "world",
    chinese = "中",
    empty = nil,
    another_empty = json.null
}

local str = json.encode(tmp, {indent = true})

print(str);

local obj, pos, err = json.decode("{\"yoxi\" : \"我是中文\"}", 1, nil);

print(obj.yoxi);
