
function generateRandomRGBAList(count) {
  var rgbaList = [];

  for (var i = 0; i < count; i++) {
    var red = Math.floor(Math.random() * 256);
    var green = Math.floor(Math.random() * 256);
    var blue = Math.floor(Math.random() * 256);

    var rgba = 'rgba(' + red + ', ' + green + ', ' + blue + ', ' + 1 + ')';
    rgbaList.push(rgba);
  }

  return rgbaList;
}

function create_chart(chart_id, labels, values, colors, title) {
    // Retrieve the data passed from Flask
    //  var labels = ['Red', 'Blue', 'Yellow'];
    //  var values = [10, 20, 30];
     console.log(labels);
     console.log(values);
     console.log(colors);
     console.log(title)
     // delete the old chart
     let char = document.getElementById(chart_id);
     let parent_char = char.parentNode;
     parent_char.removeChild(char);
     char = document.createElement("canvas");
     char.id = chart_id;
     parent_char.appendChild(char)
     // char.innerHTML = ""
     // create a new chart
     var ctx = document.getElementById(chart_id).getContext('2d');
     var myChart = new Chart(ctx, {
         type: 'pie',
         data: {
             labels: labels,
             datasets: [{
                 data: values,
                 backgroundColor: colors,
                 // backgroundColor: ['red', 'blue', 'yellow'],
             }]
         },
         options: {
            title: {
              display: false,
              text: title
            },
            plugins: {
                legend: {
                    labels: {
                        color: 'rgb(192,190,190)' // Set the desired label text color
                    }
                },
                bodyFont: {
                    color: 'rgb(192,190,190)' // Set the desired body text color
                }
        }
          }
     })
}

function get_package_payload(pack_id){
    fetch(server_location +'packages?id='+pack_id)
         .then(response => response.text())
         .then(data => {
             console.log(data)
             data = JSON.parse(data)
             //let div_content= document.getElementById('test_details');
             // div_content.innerHTML = data;
             let div_hexa = document.getElementById("payload_hexa")
             div_hexa.innerHTML= data.payload_hexa;
             let div_str = document.getElementById("payload_str")
             let ls_str_data = data.payload_str.match(/.{1,20}/g);
             div_str.innerHTML = "";
             for (let el of ls_str_data){
                 console.log(el)
                 div_str.innerHTML += el + "<br>";
             }

        }).catch(err => {
            document.getElementById('payload_hexa').textContent = err;
     })
}


function get_pack_minutes(ip,mac,number_of_minute){
    fetch(server_location +'package_clients?ip='+ip +"&mac=" +mac + "&nr=" +number_of_minute + "&type=1")
         .then(response => response.text())
         .then(data => {
             console.log(data)
             console.log(JSON.parse(data))
             let labels = []
             let values = []
             let list_stats = JSON.parse(data)
             let colors = generateRandomRGBAList(list_stats.length);
             //list_stats.sort(function(a, b){return b.value-a.value});
             list_stats= list_stats.reverse()
             let poz = 0;
             const IPPROTO = [
              { name: "IP", value: 0 },
              { name: "ICMP", value: 1 },
              { name: "IGMP", value: 2 },
              { name: "IPIP", value: 4 },
              { name: "TCP", value: 6 },
              { name: "EGP", value: 8 },
              { name: "PUP", value: 12 },
              { name: "UDP", value: 17 },
              { name: "IDP", value: 22 },
              { name: "TP", value: 29 },
              { name: "IPV6", value: 41 },
              { name: "ROUTING", value: 43 },
              { name: "FRAGMENT", value: 44 },
              { name: "RSVP", value: 46 },
              { name: "GRE", value: 47 },
              { name: "ESP", value: 50 },
              { name: "AH", value: 51 },
              { name: "ICMPV6", value: 58 },
              { name: "NONE", value: 59 },
              { name: "DSTOPTS", value: 60 },
              { name: "MTP", value: 92 },
              { name: "ENCAP", value: 98 },
              { name: "PIM", value: 103 },
              { name: "COMP", value: 108 },
              { name: "SCTP", value: 132 },
              { name: "UDPLITE", value: 136 },
              { name: "RAW", value: 255 }
            ];
             for( el of list_stats){
                 labels.push(IPPROTO.find((item) => item.value == el.t_proto)?.name);
                 values.push(el.nr_packs);
                 // poz++;
                 // if (poz >= 10){
                 //     break;
                 // }
             }
             if (labels.length !== 0) {
                 labels = labels.reverse()
                 values = values.reverse()
             }else{
                 labels.push("None");
                 values.push(0)
             }
             create_chart("canvas_packs", labels, values, colors, "");
        }).catch(err => {
            alert("Cannot retrieve last minute packs! Error :" + err);
            // document.getElementById('content').textContent = err;
     })
}

function create_packs_list(pack_list_data){
    let list_packs = document.getElementById("package_list");
    list_packs.innerHTML="";
    for (let el of pack_list_data){
        let li_pack = document.createElement("li");
        li_pack.className="li_pack";
        li_pack.innerHTML = el.details;
        li_pack.id = el.id;
        li_pack.addEventListener("click", function (event){
            let id_pack = event.target.id;
            get_package_payload(id_pack);
        });
        list_packs.appendChild(li_pack);
    }
}

function get_pack_list(ip,mac,number_of_packs){
    fetch(server_location +'package_clients?ip='+ip +"&mac=" +mac + "&nr=" +number_of_packs + "&type=2")
         .then(response => response.text())
         .then(data => {
             console.log(data)
             data = JSON.parse(data);
             create_packs_list(data);
        }).catch(err => {
            alert("Cannot retrieve packs! Error :" + err);
            // document.getElementById('content').textContent = err;
     })
}

function load_data(ip,mac){
    get_pack_minutes(ip,mac,1);
    get_pack_list(ip,mac,20);
}

function begin(){
    let dropout_container = document.getElementsByClassName("dropout")[0];
    let det = dropout_container.id;
    let ip = det.split("|")[0]
    let mac = det.split("|")[1]
    load_data(ip,mac);
    let but_refresh_pack_minute = document.getElementById("but_refresh_pack_proto");
    but_refresh_pack_minute.addEventListener("click",function (){
        let dropout_container = document.getElementsByClassName("dropout")[0];
        let det = dropout_container.id;
        let ip = det.split("|")[0]
        let mac = det.split("|")[1]
        let input_min = document.getElementById("input_minute");
        let number_of_min = input_min.value;
        if (number_of_min <= 0){
            alert(" The number of minute cannot be less than 0");
        }
        else{
            get_pack_minutes(ip,mac,number_of_min);
        }
    });
    let but_refresh_pack_list = document.getElementById("but_refresh_pack");
    but_refresh_pack_list.addEventListener("click",function (){
        let dropout_container = document.getElementsByClassName("dropout")[0];
        let det = dropout_container.id;
        let ip = det.split("|")[0]
        let mac = det.split("|")[1]
        let input_list = document.getElementById("input_list");
        let number_of_packs = input_list.value;
        if (number_of_packs <= 0){
            alert(" The number of packages cannot be less than 0");
        }
        else{
            get_pack_list(ip,mac,number_of_packs);
        }
    });
}

begin();