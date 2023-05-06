module("luci.controller.infectivity_controller.infect", package.seeall)

function index()
    entry({"admin", "network", "infectivity"}, template("infectivity_view/home"), "Infectivity", 10).dependent=false
    luci.http.header("Cache-Control", "no-cache, no-store, must-revalidate")
end



-- function action_tryme()
--     luci.http.prepare_content("text/plain")
--     luci.http.write("Haha, rebooting now...")
--     luci.sys.reboot()
-- end