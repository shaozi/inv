


angular.module('invApp', ['ngRoute', 'ui.bootstrap']);

angular.module('invApp')

.controller('invController', function($http, $rootScope, $scope, $location) {
    
    $scope.menuToggled = function(open) {
    };
    $scope.login = function() {
	if (typeof $rootScope.needlogin != 'undefined' && $rootScope.needlogin==false )
	    return;
	$http.post("auth.php",
		   {action:'login', 
		    username:$scope.username, 
		    password:$scope.password}
		  ).success(function(data, status, headers, config) {
		      console.log(data);
		      var ret = data;
		      if (ret.result == 'pass') {
			  $rootScope.needlogin = false;
			  $rootScope.firstname = ret.firstname;
			  $location.path('/');
		      } else {
			  $rootScope.needlogin = true;
			  $location.path("/login");
			  $scope.loginMessage = "Log in failed";
		      }
		  });
    };

    $scope.get_all_parts = function() {
	if (typeof $rootScope.needlogin == 'undefined' || $rootScope.needlogin )
	    return;
	$http.post("query.php", {action:'get_all_parts'})
		  .success(function(data, status, headers, config) {
		      console.log(data);
		      if (data.result == 'pass') {
 			  for (var i = 0; i< data.parts.length; i++) {
			      data.parts[i].overduedays = parseInt(data.parts[i].overduedays);
			      if (data.parts[i].status=="in") {
				  data.parts[i].customer_name = null;
				  data.parts[i].company = null;
				  data.parts[i].loandate = null;
				  data.parts[i].duebackdate = null;
				  data.parts[i].overduedays = null;
			      }
			  }
			  $rootScope.parts = data;
		      } 
		  });
    };
    $scope.get_all_parts();
})

.controller('detailController', function($http, $rootScope, $scope, $location, $routeParams) {
    
    if (typeof $rootScope.needlogin == 'undefined' || $rootScope.needlogin) {
	$location.path('/login');
	return;
    }
    $scope.serial = $routeParams.serial;

    // get part from database
    $scope.get_part_w_serial = function(serial) {
	$http.post("query.php", {action:'get_part_w_serial',
				 serial: serial})
	    .success(function(data, status, headers, config) {
		console.log(data);
		if (data.result == 'pass') {
		    data.part.overduedays = parseInt(data.part.overduedays);
		    if (data.part.status=="in") {
			data.part.customer_name = null;
			data.part.company = null;
			data.part.loandate = null;
			data.part.duebackdate = null;
			data.part.overduedays = null;
		    }
		    $scope.part = data.part;
		} else {
		    $scope.error = data;
		}
	    });
    };
    $scope.get_part_w_serial($scope.serial);
    
    $scope.checkin = function() {
	$scope.error = false;
	$http.post("query.php", {action:'checkin',
				 serial: $scope.serial})
	    .success(function(data, status, headers, config) {
		console.log(data);
		if (data.result == 'pass') {
		    $scope.get_part_w_serial($scope.serial);
		} else {
		    $scope.error = data;
		}
	    });
	
    };
    
    $scope.checkout = function() {
	$scope.error = false;
	$http.post("query.php", {action:'checkout',
				    serial: $scope.serial,
				    customer: $scope.customer.customer_name,
				    company: $scope.customer.company,
				    duebackin: $scope.duebackin,
				    comment: $scope.comment})
	    .success(function(data, status, headers, config) {
		console.log(data);
		if (data.result == 'pass') {
		    $scope.get_part_w_serial($scope.serial);
		} else {
		    $scope.error = data;
		}
	    });
    };
    $scope.gethistory = function() {
	$scope.error = false;
	$http.post("query.php", {action:'gethistory',
				 serial: $scope.serial})
	    .success(function(data, status, headers, config) {
		console.log(data);
		if (data.result == 'pass') {
		    $scope.transactions = data.transactions;
		} else {
		    $scope.error = data;
		}
	    });
    };
    
    // also get a list of customer for typeahead when checking out
    // get part from database
    $scope.getcustomer = function(input) {
	return $http.post("searchcustomername.php", {input:input})
	    .then(function(response) {
		return response.data.customers;
	    });
    };
    $scope.onselectcustomer = function($item, $model, $label) {
	$scope.customer.company = $item.company;
    };
})

.controller('logoutController', function($http, $rootScope, $scope, $location) {
    $rootScope.needlogin = true;
    $http.post("auth.php", {action:'logout'} );
})

.config(function($routeProvider, $locationProvider) {
    $routeProvider
	.when('/', {
	    templateUrl: 'main.html',
	    controller:'invController'
	})
	.when('/login', {
	    templateUrl: 'login.html',
	    controller:'invController'
	})
	.when('/logout', {
	    templateUrl: 'logout.html',
	    controller:'logoutController'
	})    
	.when('/help', {
	    templateUrl: 'help.html',
	    controller:'invController'
	})
	.when('/history/:serial', {
	    templateUrl: 'history.html',
	    controller:'invController'
	})
	.when('/edit/:serial', {
	    templateUrl: 'edit.html',
	    controller: 'invController'
	})
	.when('/detail/:serial', {
	    templateUrl: 'detail.html',
	    controller: 'detailController'
	});
})

.run(function($rootScope, $http, $location){
    if (typeof $rootScope.needlogin != 'undefined' && 
	$rootScope.needlogin==false )
	return;
    $http.get("auth.php")
	.success(function(data, status, headers, config) {
	    console.log(data);
	    var ret = data;
	    if (ret.result == 'pass') {
		$rootScope.needlogin = false;
		$rootScope.firstname = ret.firstname;
	    } else {
		$rootScope.needlogin = true;
		$location.path('/login');
	    }
	});
});
