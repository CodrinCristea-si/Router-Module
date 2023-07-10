'use restrict'

var correspondence=[
    [1,"Uninfected","-"],
    [2,"Suspicious","-"],
    [3,"Infected","Minor"],
    [4,"Infected","Major"],
    [5,"Infected","Sever"]
];

var is_automatic = true;
var is_lockdown = false;

//var global_clients_list = []
var global_id;

function create_select(values,id, default_val){
	let select=document.createElement("select");
    select.className="select";
    select.id=id;
    for (let infec of values){
        let opt= document.createElement("option");
        opt.className="option";
        opt.value=infec;
		if (infec == default_val)
			opt.selected = true;
        opt.text=infec;
        select.appendChild(opt);
    }
	return select;
}

function create_primary_select(id, default_val){
    let infectivity =[];
    for (let cor of correspondence){
        if (!(infectivity.find(inf => inf == cor[1])))
            infectivity.push(cor[1]);
    }
	//console.log("prim" + infectivity);
    return create_select(infectivity, id, default_val);
}

function create_secondary_select(primary_infec, id, default_val){
    let infectivity =[];
    //let primary_infec = correspondence.find( cor => cor[0] == type_infec);
    for (let cor of correspondence){
        if (primary_infec == cor[1] && !(infectivity.find(inf => inf == cor[2])))
            infectivity.push(cor[2]);
    }
    //console.log("second" + infectivity);
    return create_select(infectivity,id,default_val);
}

function send_to_server_save(ip,mac, state){
    // L.Request.post(window.location.href+'/transfer',{ip:ip, mac:mac, state:state},null).then(
    //     function(data){
    //         console.log("transfer sent!");
    //     });
    XHR.get(window.location.href+'/transfer', {ip:ip, mac:mac, state:state},
    function(x, info) {
            if (!info) {
                //location.href=location.href;
                console.log("transfer failed");
            }
            else {
                console.log("transfer success");
            }
        }
    );
}

function send_to_server_automatic(state){
    // L.Request.post(window.location.href+'/automatic',{state: state == true? 1:0},null).then(
    //     function(data){
    //         console.log("automatic sent!");
    //     });
    XHR.get(window.location.href+'/automatic', {state: state == true? 1:0},
    function(x, info) {
            if (!info) {
                //location.href=location.href;
                console.log("automatic failed!");
            }
            else {
                console.log("automatic success!");
            }
        }
    );
}

function send_to_server_lockdown(state){
    // L.Request.post(window.location.href+'/lockdown',{state: state == true? 1:0},null).then(
    //     function(data){
    //         console.log("lockdown sent!");
    //     });

    XHR.get(window.location.href+'/lockdown', {state: state == true? 1:0},
    function(x, info) {
            if (!info) {
                //location.href=location.href;
                console.log("lockdown failed!");
            }
            else {
                console.log("lockdown sent!");
            }
        }
    );
}

function get_id_corespondence_by_states(primary_state, secondary_state){
    state = 0;
    for (let cor of correspondence){
        if(cor[1] == primary_state && cor[2] == secondary_state){
            state = cor[0];
            break;
        }
    }
    return state;
}

function handle_save_button(event){
    let but = event.target;
    let td_actions= but.parentElement.parentElement;
    console.log(td_actions);
    let tr_client = td_actions.parentElement;
    console.log(tr_client);
    let client_id = td_actions.getAttribute("name").split("-")[1];
    let primary_state = document.getElementById("primary-"+client_id).value;
    let secondary_state = document.getElementById("secondary-"+client_id).value;
    state = get_id_corespondence_by_states(primary_state,secondary_state);
    let ip = tr_client.id.split("-")[0];
    let mac = tr_client.id.split("-")[1];
    console.log("transfer " + ip + " " + mac + " " + state);
    send_to_server_save(ip, mac, state);

} 

// function handle_automatic_ckeck(event){
//     let but = event.target;
//     let state = but.checked;
//     update_automatic_status()
// }

function transfer_client_row(client){
    let client_row = document.getElementById(client.ip+"-"+client.mac)
    let td_client = client_row.getAttribute("name");
    if (!td_client || td_client == null){
        return;
    }
    let client_id = td_client.split("-")[2]

    let cor = correspondence.find(cor => cor[0] == client.infect);
    let type_infec=cor[0];
    let infec_pr=cor[1];
    let infec_sec=cor[2];


    //div for selects
    let div3 = document.getElementById(client_id);
    console.log(div3);
    //select infectivity primary
    let select1 = document.getElementById("primary-"+client_id);
    let select2 = document.getElementById("secondary-"+client_id);
    //div3.removeChild(select2);
    select2.parentNode.removeChild(select2);
    select1.value = infec_pr;
    //select for infectivity secondary
    select2 = create_secondary_select(infec_pr,"secondary-"+client_id, infec_sec);
    select2.value=infec_sec;
    select1.parentNode.appendChild(select2);
    //div3.appendChild(select2);

}

function delete_client_row(client){
    let list_clients = document.getElementById("list_clients");
    let client_row = document.getElementById(client.ip+"-"+client.mac)
    list_clients.removeChild(client_row)
}

function create_client_row(client){
    let cl_elm = document.createElement("tr");
    if(cl_elm){
        cl_elm.className="tr cbi-rowstyle-1";
        cl_elm.setAttribute("id",client.ip + "-"+client.mac)
        cl_elm.setAttribute("name",client.ip + "-"+client.mac+"-"+ client.id)
        //for ip 
        let attr_elem1 = document.createElement("td");
        attr_elem1.className="td";
        attr_elem1.setAttribute("name","IP-"+client.id)
        attr_elem1.innerHTML = client.ip;
        cl_elm.appendChild(attr_elem1);

        //for mac
        let attr_elem2 = document.createElement("td");
        attr_elem2.className="td";
        attr_elem2.setAttribute("name","MAC-"+client.id)
        attr_elem2.innerHTML = client.mac;
        cl_elm.appendChild(attr_elem2);

        //for infectivity
        let cor = correspondence.find(cor => cor[0] == client.infect);
        let type_infec=cor[0];
        let infec_pr=cor[1];
        let infec_sec=cor[2];

        //for infectivity types
        let attr_elem3 = document.createElement("td");
        attr_elem3.className="td";
        attr_elem3.setAttribute("name","STATE-"+client.id)

        //div for selects
        let div3 = document.createElement("div");
        div3.id = client.id

        //select infectivity primary
        let select1 = create_primary_select("primary-"+client.id, infec_pr);
        
        select1.addEventListener("change",((event)=>{
            console.log("event");
            let id_sel = "secondary-" + event.target.id.split('-')[1];
            console.log(id_sel);
            let select =document.getElementById(id_sel);
            console.log(select);
            let parent = select.parentElement;
            parent.removeChild(select);
            let select_new = create_secondary_select(event.target.value, id_sel,"");
            parent.appendChild(select_new);
        }));
        div3.appendChild(select1);
        
        //this cause firefox to f**k the first select, idk why, ask W3C
        //div3.innerHTML += "<br>";
        //attr_elem3.style.display = "flex";
        //div3.style.display = "flex";

        //select for infectivity secondary
        let select2 = create_secondary_select(infec_pr,"secondary-"+client.id, infec_sec);
        select2.value=infec_sec;
        div3.appendChild(select2);
        attr_elem3.appendChild(div3);
        cl_elm.appendChild(attr_elem3);

        //for actions
        let attr_elem4 = document.createElement("td");
        attr_elem4.className="td";
        attr_elem4.setAttribute("name","ACTIONS-"+client.id)

        let div4 = document.createElement("div");
        //button save
        let but_save = document.createElement("button")
        but_save.addEventListener("click",function(event){
            console.log("save");
            handle_save_button(event);
        })
        but_save.className="cbi-button cbi-button-edit";
        but_save.innerHTML="Save";
        //button test
        // let but_test = document.createElement("button")
        // but_test.addEventListener("click",function(){
        //     console.log("test");
        // })
        // but_test.className="cbi-button cbi-button-edit";
        // but_test.innerHTML="Test";
        // div4.appendChild(but_test);
        div4.appendChild(but_save);
        attr_elem4.appendChild(div4);

        cl_elm.appendChild(attr_elem4);
    }
    return cl_elm
}

function add_client_row(client){
    let list_clients = document.getElementById("list_clients");
    let client_row = document.getElementById(client.ip+"-"+client.mac)
    if (client_row){
        list_clients.removeChild(client_row);
    }
    let row = create_client_row(client);
    list_clients.appendChild(row);
    
}

function create_clients_table(list_cl){
    //console.log(list_cl);
    let list_clients = document.getElementById("list_clients");
    //list_clients.innerHTML="";
    for(let client of list_cl){
        //console.log(client);
        add_client_row(client);
    }
}

function update_lockdown_button_automatic(is_checked){
    if (is_automatic){
        let div_cont = document.getElementById("slider_container");
        div_cont.classList.remove('disabled');
        div_cont.checked = is_checked;
        div_cont.classList.add('disabled');
    }
}

function update_lockdown_status(new_status){
    is_lockdown = new_status== 1 ? true : false;
    update_lockdown_button_automatic(is_lockdown);
    if(is_lockdown)
        alert("The network will be in lockdown mode!");
    //send to server
}

function update_automatic_status(new_status){
    is_automatic = new_status == 1 ? true : false;
    if (new_status){
        console.log("Checkbox is checked..");
        let div_cont = document.getElementById("slider_container");
        div_cont.classList.add('disabled');
    } else {
        let div_cont = document.getElementById("slider_container");
        div_cont.classList.remove('disabled');
        console.log("Checkbox is not checked..");
    }
    let but_auto = document.getElementById("automaticCheckBox");
    but_auto.checked = new_status;
    //send to server
}

function update_automatic_status_button(new_state){
    let automaticCheck = document.getElementById("automaticCheckBox");
    automaticCheck.checked = new_state;
    is_automatic = new_state;
    if (new_state == false){
        let div_cont = document.getElementById("slider_container");
        div_cont.classList.remove('disabled');
    }
}

function update_lockdown_status_button(new_state){
    let toogleBut = document.getElementById("toggleButton");
    toogleBut.checked = new_state;
}

function load_data(){
    L.Request.get(window.location.href+'/load',null).then(
        function(data){
            var list_clients=[];
            console.log(data);
            console.log(data.responseText);
            console.log(data.responseText.slice(2,-2).replaceAll("\\n","").replaceAll("\\",""));
            if(data.responseText.slice(2,-2).replaceAll("\\n","").replaceAll("\\","")){
                let payload = JSON.parse(data.responseText.slice(2,-2).replaceAll("\\n","").replaceAll("\\",""))
                let ls_cl= payload["data"];
                console.log(ls_cl);
                for (let el of ls_cl){
                    if (el["lockdown"]){
                        console.log("lock");
                        update_lockdown_status(el["lockdown"]);
                        update_lockdown_status_button(el["lockdown"] == 1 ? true : false);
                    }
                    else if (el["automatic"]){
                        console.log("auto");
                        update_automatic_status(el["automatic"]);
                        update_automatic_status_button(el["automatic"] == 1 ? true : false);
                    }
                    else{
                        let client = {id: "",ip:"", mac:"",infect:""}
                        client.ip=el.ip;
                        client.mac=el.mac;
                        client.infect=el.infect;
                        client.id=global_id
                        global_id+=1
                        if (client.ip && client.ip != "")
                            list_clients.push(client);
                    }
                }
                //global_clients_list = list_clients;
                console.log(list_clients);
                create_clients_table(list_clients);
            }
        });
}

function load_updates(){
    L.Request.get(window.location.href+'/updates',null).then(
        function(data){
            console.log(data);
            console.log(data.responseText);
            console.log(data.responseText.slice(2,-2).replaceAll("\\n","").replaceAll("\\",""));
            if (data.responseText.slice(2,-2).replaceAll("\\n","").replaceAll("\\","")){
                try{
                    let payload = JSON.parse(data.responseText.slice(2,-2).replaceAll("\\n","").replaceAll("\\",""))
                    let ls_cl= payload["updates"];
                    console.log(ls_cl);
                    for (let el of ls_cl){
                        if (el["lockdown"]){
                            update_lockdown_status(el["lockdown"]);
                            update_lockdown_status_button(el["lockdown"]== 1 ? true : false);
                            
                        }
                        else if (el["automatic"]){
                            update_automatic_status(el["automatic"]);
                            update_automatic_status_button(el["automatic"] == 1 ? true : false);
                        }
                        else{
                            let client = {type: "", ip:"", mac:"",infect:""}
                            client.ip=el.ip;
                            client.mac=el.mac;
                            client.infect=el.infect;
                            if (el.type == 1){
                                client.id=global_id
                                global_id+=1
                                add_client_row(client)
                            }
                            else if (el.type == 2){
                                delete_client_row(client)
                            }
                            else if(el.type == 3){
                                transfer_client_row(client)
                            }
                        }
                    }
                }
                catch(err){
                    console.error(err);
                }
            }
        });
}

function execute_start(){
    // let cl1 = {id:"1", ip:"192.168.1.23", mac:"a5:45:78:e4:0b:16",infect:"1"};
    // let cl2 = {id:"2", ip:"192.168.1.46", mac:"67:45:89:a4:db:23",infect:"2"};
    // let cl3 = {id:"3", ip:"192.168.1.100", mac:"45:45:45:45:45:45",infect:"3"};
    // let cl4 = {id:"4", ip:"192.168.1.56", mac:"12:12:12:12:12:12",infect:"4"};
    // let cl5 = {id:"5", ip:"192.168.1.76", mac:"cd:cd:dc:dc:cd:cd",infect:"5"};
	// let list_clients =[cl1,cl2,cl3,cl4,cl5];
    // lobal_id = 6;
    // create_clients_table(list_clients);

	global_id = 0;
    load_data();
    
}

console.log("start");
window.onload=(()=>{
    let automaticCheck = document.getElementById("automaticCheckBox");
    automaticCheck.addEventListener('change', function() {
        update_automatic_status(this.checked);
        send_to_server_automatic(this.checked);
    });

    let lockdownCheck = document.getElementById("toggleButton");
    lockdownCheck.addEventListener('change', function() {
        update_lockdown_status(this.checked);
        send_to_server_lockdown(this.checked);
    });
    //lockdownCheck.checked = false;
    //update_automatic_status(true);
    
    is_automatic = true;
    is_lockdown = false;
    // var toggleButton = document.getElementById('toggleButton');
    // toggleButton.addEventListener('change', function() {
    //     var sliderText = document.getElementById('sliderText');
    //     sliderText.innerHTML = this.checked ? 'ON' : 'OFF';
    // });
    execute_start();
});
const interval = 3000
window.setInterval(() => {
    //load_data()
    load_updates()
}, interval);
