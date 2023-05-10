module("luci.controller.infectivity_controller.infect", package.seeall)



function index()
    --load html file
    entry({"admin", "network", "infectivity"}, template("infectivity_view/home"), "Infectivity", 10).dependent=false

    --load js script
    page = entry({"admin", "network", "infectivity", "script"}, call("send_scripts"), nill)
    page.leaf=true

    --get data
    page = entry({"admin", "network", "infectivity", "load"}, call("send_data"), nill)
    page.leaf=true
    
    luci.http.header("Cache-Control", "no-cache, no-store, must-revalidate")
    -- luci.http.header('Content-Type', 'application/javascript')
end

function send_data()
    local decoy ={}
    decoy["data"]={}
    decoy["data"][1]={}
    decoy["data"][1]["ip"]="192.168.1.23"
    decoy["data"][1]["mac"]="a5:45:78:e4:0b:16"
    decoy["data"][1]["infect"]="1"

    decoy["data"][2]={}
    decoy["data"][2]["ip"]="192.168.1.46"
    decoy["data"][2]["mac"]="67:45:89:a4:db:23"
    decoy["data"][2]["infect"]="2"

    decoy["data"][3]={}
    decoy["data"][3]["ip"]="192.168.1.100"
    decoy["data"][3]["mac"]="45:45:45:45:45:45"
    decoy["data"][3]["infect"]="3"

    decoy["data"][4]={}
    decoy["data"][4]["ip"]="192.168.1.56"
    decoy["data"][4]["mac"]="12:12:12:12:12:12"
    decoy["data"][4]["infect"]="4"

    decoy["data"][5]={}
    decoy["data"][5]["ip"]="192.168.1.76"
    decoy["data"][5]["mac"]="cd:cd:dc:dc:cd:cd"
    decoy["data"][5]["infect"]="5"

    luci.http.prepare_content("application/json")
    luci.http.write_json(decoy)

end

function send_scripts()
    local path_script = "/usr/lib/lua/luci/view/infectivity_view/files/script.js"
    local file = io.open(path_script,"r");
    local script_content = "";
    if file~=nil then
        script_content = file:read("*all");
        io.close(file)
    end
    luci.http.prepare_content("application/javascript")
    luci.http.write(script_content)
end

-- function load_page()
--     template("infectivity_view/files-home-script")
-- end