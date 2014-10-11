


angular.module('invApp', ['ngRoute', 'ui.bootstrap']);

angular.module('invApp')

.controller('invController', function($http, $rootScope, $scope, $location) {
    
    $scope.menuToggled = function(open) {
    };
    $scope.login = function() {
	if (typeof $rootScope.needlogin != 'undefined' && $rootScope.needlogin==false )
	    return;
	$http.post("inv.php",
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
    $scope.login();
})

.controller('logoutController', function($http, $rootScope, $scope, $location) {
    
    $http.post("inv.php",
	       {action:'logout'}
	      ).success(function(data, status, headers, config) {
		  console.log(data);
		  $rootScope.needlogin = true;
	      });
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
	    controller: 'invController'
	});
});
