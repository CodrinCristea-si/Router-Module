'use restrict'
'require ui'
'require view'
'require dom'
// L.require("ui").then(()=>{
//     //new LuCI.ui();
//     L.require("dom").then(()=>{
//         //new LuCI.dom();
//         execute_start()
//     })
// });
// L.require('dom').then(()=>{
//     console.log("dom");
// })
// L.require('ui').then(()=>{
//     console.log("ui");
// })
// L.dom.byId = function(id) {
//   return document.getElementById(id);
// };

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
    var select=E("select",{"class":"select", "id":id},[]);
    for (let infec of infectivity){
        var opt= E("option",{"class":"option", "value":infec},[infec]);
        select.appendChild(opt)
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
    var select=E("select",{"class":"select", "id":id},[]);
    for (let infec of infectivity){
        var opt= E("option",{"class":"option", "value":infec},[infec]);
        select.appendChild(opt)
    }
    return select;
}

function create_list_table_view(list_cl){
    var list_clients= E("table",{"class":"table","id":"list_clients"},[]);
    var row_header = E("tr",{"class":"tr table-titles"},[]);
    row_header.appendChild(E("th",{"class":"th nowrap","data-sortable-row":"true"},"IP Address"));
    row_header.appendChild(E("th",{"class":"th hide-xs","data-sortable-row":"true"},"MAC Address"));
    row_header.appendChild(E("th",{"class":"th ","data-sortable-row":"true"},"Infectivity"));
    row_header.appendChild(E("th",{"class":"th ",},"Actions"));
    list_clients.appendChild(row_header);
    let line_nr = 0;
    for(let client of list_cl){
        console.log(client);
        line_nr++;
        var cl_elm = E("tr",{"class":"tr cbi-rowstyle-1"},[]);
        if(cl_elm){
            //for ip 
            var attr_elem1 = E("td",{"class":"td"},client.ip);
            cl_elm.appendChild(attr_elem1);

            //for mac
            var attr_elem2 = E("td",{"class":"td"},client.mac);
            cl_elm.appendChild(attr_elem2);

            //for infectivity
            var cor = correspondence.find(cor => cor[0] == client.infect);
            var type_infec=cor[0], infec_pr=cor[1], infec_sec=cor[2];

            console.log(type_infec, infec_pr, infec_sec);

            let attr_elem3 = E("td",{"class":"td"},[]);

            let select1 = create_primary_select("primary-"+line_nr);
            select1.innerHTML+=infec_pr;
            //rework
            select1.on("change",((event)=>{
            console.log("event");
            let id_sel = event.currentTarger.id.split('-')[1];
                let select = L.dom.byId("secondary" + id_sel);
                let parent = select.parentNode();
                let select_new = create_secondary_select(value, select.id);

                parent.removeChild(select);
                parent.appendChild(select_new);
            }))
            //console.log(select1.value, infec_pr);
            attr_elem3.appendChild(select1);
            attr_elem3.innerHTML += "<br><br>";

            let select2 = create_secondary_select(infec_pr,"secondary-"+line_nr);
            select2.innerHTML+=infec_sec;
            attr_elem3.appendChild(select2);

            cl_elm.appendChild(attr_elem3);

            //for actions
            let attr_elem4 = E("td",{"class":"td"},[]);
            let but = E("button",{"class":"cbi-button cbi-button-edit"})
            but.innerHTML="Save";
            attr_elem4.appendChild(but);
            cl_elm.appendChild(attr_elem4);
            list_clients.appendChild(cl_elm);
        }
    }
    return list_clients;
}

return view.extend({

    load:function(){
        return Promise.all(async function(){
            console.log("Loading data...");
            var cl1 = {ip:"192.168.1.23", mac:"a5:45:78:e4:0b:16",infect:"1"};
            var cl2 = {ip:"192.168.1.46", mac:"67:45:89:a4:db:23",infect:"2"};
            var cl3 = {ip:"192.168.1.100", mac:"45:45:45:45:45:45",infect:"3"};
            var cl4 = {ip:"192.168.1.56", mac:"12:12:12:12:12:12",infect:"4"};
            var cl5 = {ip:"192.168.1.76", mac:"cd:cd:dc:dc:cd:cd",infect:"5"};
            var list_cl=[cl1,cl2,cl3,cl4,cl5];
            return list_cl;
        }
        ).then(function(list_cl){
            return list_cl;
        });
    },
    render:function(list_cl){
        var main_div= E("div",[]);
        var list_table = create_list_table_view(list_cl);
        main_div.appendChild(list_table);
        return main_div;
    },

    handleSaveApply: null,
	handleSave: null,
	handleReset: null

});
        