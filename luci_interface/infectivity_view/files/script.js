'use restrict'

var correspondence=[
    [1,"Uninfected","-"],
    [2,"Suspicious","-"],
    [3,"Infected","Minor"],
    [4,"Infected","Major"],
    [5,"Infected","Sever"]
];

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

function update_clients(list_cl){
    //console.log(list_cl);
    let list_clients = document.getElementById("list_clients");
    let line_nr = 0;
    for(let client of list_cl){
        //console.log(client);
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

            // console.log(type_infec, infec_pr, infec_sec);

            //for infectivity types
            let attr_elem3 = document.createElement("td");
            attr_elem3.className="td";
            //div for selects
			let div3 = document.createElement("div");
            //div3.className="select-container"
            //select infectivity primary
            let select1 = create_primary_select("primary-"+line_nr, infec_pr);
            
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
            // console.log(select1.value, infec_pr);
            // console.log(select1);
            div3.appendChild(select1);
            //select1.onchange=function(){console.log("salut")}
            
            //this cause mozzilla to f**k the first select, idk why, ask W3WC
            //div3.innerHTML += "<br>";
            //attr_elem3.style.display = "flex";
            //div3.style.display = "flex";

            //select for infectivity secondary
            let select2 = create_secondary_select(infec_pr,"secondary-"+line_nr, infec_sec);
            select2.value=infec_sec;
            div3.appendChild(select2);
            select2.onchange=function(){console.log("salut2")};
			attr_elem3.appendChild(div3);

            cl_elm.appendChild(attr_elem3);
            //for actions
            let attr_elem4 = document.createElement("td");
            attr_elem4.className="td";
            let but = document.createElement("button")
			but.addEventListener("click",function(){
				console.log("click");
			})
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
            //console.log(data);
            let payload = data.json()
            let ls_cl= payload["data"];
            //console.log(ls_cl);
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
	// let list_cl =[cl1,cl2,cl3,cl4,cl5];
    load_data();
	//update_clients(list_cl);
    
}

console.log("start");
window.onload=(()=>{
    execute_start();
});
