function begin(){
   let listItems = document.querySelectorAll('.list li .clickeble_item');
  console.log(listItems)
  listItems.forEach(function(item) {
    item.addEventListener('click', function() {
      const content = this.nextElementSibling;
      content.style.display = (content.style.display === 'none') ? 'block' : 'none';
      console.log("click");
    });
  });
  let listjson = document.querySelectorAll('.json-content');
  listjson.forEach(function(item) {
    data = item.innerHTML.replaceAll("\'","\"",);
    data_obj = JSON.parse(data)
    item.innerHTML = JSON.stringify(data_obj, undefined, 2);
    // hljs.highlightElement(item);
  });
}
begin();