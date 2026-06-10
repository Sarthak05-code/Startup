const obj = {
    name : "Sarthak",
    age : 20 , 
    class : "BCA"
}

console.log(obj);

const objArray = {
     array : [1,2,3,4,5],
     returnIndex : function(i) {
        return objArray.array[i]
     }
     
}
objArray.array.push(6)


console.log(objArray.array);

objArray.array.pop()

console.log(objArray.array);

console.log("The index is : ", objArray.returnIndex(3));






