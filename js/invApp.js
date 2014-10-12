


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
		      }
		  });
    };

    $scope.get_all_parts = function() {
	if (typeof $rootScope.needlogin == 'undefined' || $rootScope.needlogin )
	    return;
	$http.post("query.php", {action:'query'})
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
    var parts = null;
    var part = null;
    if (typeof $rootScope.parts == 'object') {
	parts = $rootScope.parts.parts;
	for (var i = 0; i< parts.length; i++) {
	    console.log(parts[i].serial);
	    if (parts[i].serial == $scope.serial) {
		console.log("found");
		$scope.part = parts[i];
		break;
	    }
	}
    }
    
    if (part == null) {
	// get part from database
    }
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
