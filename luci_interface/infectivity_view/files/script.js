'use restrict'

var correspondence=[
    [1,"Uninfected","-"],
    [2,"Suspicious","-"],
    [3,"Infected","Minor"],
    [4,"Infected","Major"],
    [5,"Infected","Sever"]
];

function create_primary_select(id){
    let infectivity =[];
    for (let cor of correspondence){
        if (!(infectivity.find(inf => inf == cor[1])))
            infectivity.push(cor[1]);
    }
    let select=document.createElement("select");
    select.className="select";
    select.id=id;
    for (let infec of infectivity){
        let opt= document.createElement("option");
        opt.className="option";
        opt.value=infec;
        opt.text=infec;
        select.appendChild(opt);
    }
    return select;
}

function create_secondary_select(primary_infec, id){
    let infectivity =[];
    //let primary_infec = correspondence.find( cor => cor[0] == type_infec);
    for (let cor of correspondence){
        if (primary_infec == cor[1] && !(infectivity.find(inf => inf == cor[2])))
            infectivity.push(cor[2]);
    }
    let select=document.createElement("select");
    select.className="select";
    select.id=id;
    for (let infec of infectivity){
        let opt= document.createElement("option");
        opt.className="option";
        opt.value=infec;
        opt.text=infec;
        select.appendChild(opt);
    }
    return select;
}

function update_clients(list_cl){
    console.log(list_cl);
    let list_clients = document.getElementById("list_clients");
    let line_nr = 0;
    for(let client of list_cl){
        console.log(client);
        line_nr++;
        let cl_elm = document.createElement("tr");
        cl_elm.className="tr cbi-rowstyle-1";
        if(cl_elm){
            //for ip 
            let attr_elem1 = document.createElement("td");
            attr_elem1.className="td";
            attr_elem1.innerHTML = client.ip;
            cl_elm.appendChild(attr_elem1);

            //for mac
            let attr_elem2 = document.createElement("td");
            attr_elem2.className="td";
            attr_elem2.innerHTML = client.mac;
            cl_elm.appendChild(attr_elem2);

            //for infectivity
            let cor = correspondence.find(cor => cor[0] == client.infect);
            let type_infec=cor[0];
            let infec_pr=cor[1];
            let infec_sec=cor[2];

            console.log(type_infec, infec_pr, infec_sec);

            let attr_elem3 = document.createElement("td");
            attr_elem3.className="td";

            let select1 = create_primary_select("primary-"+line_nr);
            
            select1.value=infec_pr;
            //select1.text=infec_pr
            //rework
            select1.addEventListener("change",((event)=>{
                console.log("event");
                let id_sel = event.target.id.split('-')[1];
                let select =document.getElementById("secondary" + id_sel);
                let parent = select.parentNode();
                let select_new = create_secondary_select(value, select.id);

                parent.removeChild(select);
                parent.appendChild(select_new);
            }));
            console.log(select1.value, infec_pr);
            console.log(select1);
            attr_elem3.appendChild(select1);
            select1.onchange=function(){console.log("salut")}

            attr_elem3.innerHTML += "<br><br>";

            let select2 = create_secondary_select(infec_pr,"secondary-"+line_nr);
            select2.value=infec_sec;
            attr_elem3.appendChild(select2);
            select2.onchange=function(){console.log("salut2")}

            cl_elm.appendChild(attr_elem3);
            //for actions
            let attr_elem4 = document.createElement("td");
            attr_elem4.className="td";
            let but = document.createElement("button")
            but.className="cbi-button cbi-button-edit";
            but.innerHTML="Save";
            attr_elem4.appendChild(but);
            cl_elm.appendChild(attr_elem4);
            
            list_clients.appendChild(cl_elm);
        }
    }
}

function load_data(){
    L.Request.get(window.location.href+'/load',null).then(
        function(data){
            var list_clients=[];
            console.log(data);
            let payload = data.json()
            let ls_cl= payload["data"];
            console.log(ls_cl);
            for (let el of ls_cl){
                let client = {ip:"", mac:"",infect:""}
                client.ip=el.ip;
                client.mac=el.mac;
                client.infect=el.infect;
                list_clients.push(client);
            }
            update_clients(list_clients);
        });
}

function execute_start(){
    // let cl1 = {ip:"192.168.1.23", mac:"a5:45:78:e4:0b:16",infect:"1"};
    // let cl2 = {ip:"192.168.1.46", mac:"67:45:89:a4:db:23",infect:"2"};
    // let cl3 = {ip:"192.168.1.100", mac:"45:45:45:45:45:45",infect:"3"};
    // let cl4 = {ip:"192.168.1.56", mac:"12:12:12:12:12:12",infect:"4"};
    // let cl5 = {ip:"192.168.1.76", mac:"cd:cd:dc:dc:cd:cd",infect:"5"};
    load_data();
    
}

console.log("start");
window.onload=(()=>{
    execute_start();
});


// <!-- <script src="/luci-static/resources/luci.js"></script> -->
// <!-- <script type="text/javascript" src="<%=luci.dispatcher.build_url('network/infectivity/home-script.js')%>"> </script> -->
// <!-- <script type="text/javascript" src="/view/network/infectivity/home-script.js"></script> -->
// <!-- <%=luci.util.lib("luci-static.resources.my_script")%> -->

// <!-- <div class="network-status-table">
//     <div class="ifacebox">
//         <div class="ifacebox-head center active">
//             <strong>radio0</strong>
//         </div>
//         <div class="ifacebox-body left">
//             <span>
//                 <span class="nowrap">
//                     <strong>Type: 

//                     </strong>MediaTek MT76x8 802.11bgn</span><br>
//                     <span class="nowrap">
//                         <strong>Channel: </strong>
//                         13 (2.472 GHz)
//                     </span><br>
//                     <span class="nowrap"><strong>Bitrate: </strong>54 Mbit/s</span></span><div><span class="ifacebadge"><img src="/luci-static/resources/icons/signal-75-100.png" title="Signal: -24 dBm / Quality: 100%"><span><span class="nowrap"><strong>SSID: </strong>OpenWrt</span><br><span class="nowrap"><strong>Mode: </strong>Master</span><br><span class="nowrap"><strong>BSSID: </strong>32:DE:4B:5A:BB:CC</span><br><span class="nowrap"><strong>Encryption: </strong>WPA2 PSK (CCMP)</span><br><span class="nowrap"><strong>Associations: </strong>1</span><br></span></span><span class="ifacebadge"><img src="/luci-static/resources/icons/signal-75-100.png" title="Signal: -25 dBm / Quality: 100%"><span><span class="nowrap"><strong>SSID: </strong>Parola este 35789473</span><br><span class="nowrap"><strong>Mode: </strong>Client</span><br><span class="nowrap"><strong>BSSID: </strong>30:DE:4B:5A:BB:CC</span><br><span class="nowrap"><strong>Encryption: </strong>WPA2 PSK (CCMP)</span><br><span class="nowrap"><strong>Associations: </strong>1</span><br></span></span></div></div></div></div>
//                     <h3>Associated Stations</h3>
//                     <table class="table assoclist" data-idref="73bd1ea2">
//                         <tr class="tr table-titles">
//                             <th class="th nowrap" data-sortable-row="true">Network</th>
//                             <th class="th hide-xs" data-sortable-row="true">MAC address</th>
//                             <th class="th" data-sortable-row="true">Host</th>
//                             <th class="th" data-sortable-row="true">Signal / Noise</th>
//                             <th class="th" data-sortable-row="true">RX Rate / TX Rate</th>
//                             <th class="th cbi-section-actions"></th>
//                         </tr>
//                         <tr class="tr cbi-rowstyle-1">
//                             <td class="td nowrap" data-title="Network">
//                                 <span class="ifacebadge" title="Wireless Network: Master &quot;OpenWrt&quot; (wlan0-1)" data-ifname="wlan0-1" data-ssid="OpenWrt">
//                                     <img src="/luci-static/resources/icons/wifi.png">
//                                     <span> Master "OpenWrt"<small> (wlan0-1)</small></span>
//                                 </span>
//                             </td>
//                             <td class="td hide-xs" data-title="MAC address">BC:D0:74:84:95:78</td>
//                             <td class="td" data-title="Host">Cristeas-MBP.lan <span class="hide-xs">(192.168.1.123, fe80::c1b:698c:2c56:9dd)</span></td>
//                             <td class="td" data-title="Signal / Noise">
//                                 <span class="ifacebadge" title="Signal: -24 dBm" data-signal="-24" data-noise="0">
//                                     <img src="/luci-static/resources/icons/signal-75-100.png">
//                                     <span> -24&nbsp;dBm</span>
//                                 </span>
//                             </td>
//                             <td class="td" data-title="RX Rate / TX Rate">
//                                 <span>
//                                     <span>54.0&nbsp;Mbit/s, 20&nbsp;MHz</span><br>
//                                     <span>54.0&nbsp;Mbit/s, 20&nbsp;MHz</span>
//                                 </span>
//                             </td>
//                             <td class="td cbi-section-actions">
//                                 <button class="cbi-button cbi-button-remove">Disconnect</button>
//                             </td>
//                         </tr>


//                         <tr class="tr cbi-rowstyle-2"><td class="td nowrap" data-title="Network"><span class="ifacebadge" title="Wireless Network: Client &quot;Parola este 35789473&quot; (wlan0)" data-ifname="wlan0" data-ssid="Parola este 35789473"><img src="/luci-static/resources/icons/wifi.png"><span> Client "Parola este 35789473"<small> (wlan0)</small></span></span></td><td class="td hide-xs" data-title="MAC address">DC:9B:D6:B5:62:1C</td><td class="td" data-title="Host">?</td><td class="td" data-title="Signal / Noise"><span class="ifacebadge" title="Signal: -26 dBm" data-signal="-26" data-noise="0"><img src="/luci-static/resources/icons/signal-75-100.png"><span> -26&nbsp;dBm</span></span></td><td class="td" data-title="RX Rate / TX Rate"><span><span>144.4&nbsp;Mbit/s, 20&nbsp;MHz, MCS&nbsp;15, Short&nbsp;GI</span><br><span>144.4&nbsp;Mbit/s, 20&nbsp;MHz, MCS&nbsp;15, Short&nbsp;GI</span></span></td><td class="td cbi-section-actions">-</td></tr></table>



//                     <div class="network-status-table"><div class="ifacebox"><div class="ifacebox-head center active"><strong>radio0</strong></div><div class="ifacebox-body left"><span><span class="nowrap"><strong>Type: </strong>MediaTek MT76x8 802.11bgn</span><br><span class="nowrap"><strong>Channel: </strong>13 (2.472 GHz)</span><br><span class="nowrap"><strong>Bitrate: </strong>54 Mbit/s</span></span><div><span class="ifacebadge"><img src="/luci-static/resources/icons/signal-75-100.png" title="Signal: -25 dBm / Quality: 100%"><span><span class="nowrap"><strong>SSID: </strong>OpenWrt</span><br><span class="nowrap"><strong>Mode: </strong>Master</span><br><span class="nowrap"><strong>BSSID: </strong>32:DE:4B:5A:BB:CC</span><br><span class="nowrap"><strong>Encryption: </strong>WPA2 PSK (CCMP)</span><br><span class="nowrap"><strong>Associations: </strong>1</span><br></span></span><span class="ifacebadge"><img src="/luci-static/resources/icons/signal-75-100.png" title="Signal: -26 dBm / Quality: 100%"><span><span class="nowrap"><strong>SSID: </strong>Parola este 35789473</span><br><span class="nowrap"><strong>Mode: </strong>Client</span><br><span class="nowrap"><strong>BSSID: </strong>30:DE:4B:5A:BB:CC</span><br><span class="nowrap"><strong>Encryption: </strong>WPA2 PSK (CCMP)</span><br><span class="nowrap"><strong>Associations: </strong>1</span><br></span></span></div></div></div></div><h3>Associated Stations</h3><table class="table assoclist" data-idref="5dcb517d"><tr class="tr table-titles"><th class="th nowrap" data-sortable-row="true">Network</th><th class="th hide-xs" data-sortable-row="true">MAC address</th><th class="th" data-sortable-row="true">Host</th><th class="th" data-sortable-row="true">Signal / Noise</th><th class="th" data-sortable-row="true">RX Rate / TX Rate</th><th class="th cbi-section-actions"></th></tr><tr class="tr cbi-rowstyle-1"><td class="td nowrap" data-title="Network"><span class="ifacebadge" title="Wireless Network: Master &quot;OpenWrt&quot; (wlan0-1)" data-ifname="wlan0-1" data-ssid="OpenWrt"><img src="/luci-static/resources/icons/wifi.png"><span> Master "OpenWrt"<small> (wlan0-1)</small></span></span></td><td class="td hide-xs" data-title="MAC address">BC:D0:74:84:95:78</td><td class="td" data-title="Host">Cristeas-MBP.lan <span class="hide-xs">(192.168.1.123, fe80::c1b:698c:2c56:9dd)</span></td><td class="td" data-title="Signal / Noise"><span class="ifacebadge" title="Signal: -24 dBm" data-signal="-24" data-noise="0"><img src="/luci-static/resources/icons/signal-75-100.png"><span> -24&nbsp;dBm</span></span></td><td class="td" data-title="RX Rate / TX Rate"><span><span>54.0&nbsp;Mbit/s, 20&nbsp;MHz</span><br><span>54.0&nbsp;Mbit/s, 20&nbsp;MHz</span></span></td><td class="td cbi-section-actions"><button class="cbi-button cbi-button-remove">Disconnect</button></td></tr><tr class="tr cbi-rowstyle-2"><td class="td nowrap" data-title="Network"><span class="ifacebadge" title="Wireless Network: Client &quot;Parola este 35789473&quot; (wlan0)" data-ifname="wlan0" data-ssid="Parola este 35789473"><img src="/luci-static/resources/icons/wifi.png"><span> Client "Parola este 35789473"<small> (wlan0)</small></span></span></td><td class="td hide-xs" data-title="MAC address">DC:9B:D6:B5:62:1C</td><td class="td" data-title="Host">?</td><td class="td" data-title="Signal / Noise"><span class="ifacebadge" title="Signal: -28 dBm" data-signal="-28" data-noise="0"><img src="/luci-static/resources/icons/signal-75-100.png"><span> -28&nbsp;dBm</span></span></td><td class="td" data-title="RX Rate / TX Rate"><span><span>144.4&nbsp;Mbit/s, 20&nbsp;MHz, MCS&nbsp;15, Short&nbsp;GI</span><br><span>144.4&nbsp;Mbit/s, 20&nbsp;MHz, MCS&nbsp;15, Short&nbsp;GI</span></span></td><td class="td cbi-section-actions">-</td></tr></table> -->