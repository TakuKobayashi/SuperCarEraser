var express = require('express');
var router = express.Router();

/* GET home page. */
router.get('/', function(req, res, next) {
  console.log("Index");
  // res.render('index', { title: 'Express' });
});

router.get('/red', function(req, res, next) {
  console.log("GET request from Red");
  // res.render('index', { title: 'Express' });
});

router.get('/blue', function(req, res, next) {
  console.log("GET request from Blue");
  // res.render('index', { title: 'Express' });
});



/* POST home page. */
router.post('/', function(req, res, next) {
  console.log("Index");
  // res.render('index', { title: 'Express' });
});

// app.post('red', function (req, res) {
//   console.log("POST request from Red");
//   res.send(req.body);
// });

// app.post('blue', function (req, res) {
//   console.log("POST request from Blue");
//   res.send(req.body);
// });






module.exports = router;
