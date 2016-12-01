CONTENTS OF THIS FILE
---------------------
   
 * Introduction
 * Requirements
 * Recommended modules
 * Installation
 * Configuration
 * Troubleshooting
 * FAQ
 * Maintainers


INTRODUCTION
------------

Current Maintainer: Luke Leber <lukeleber@gmail.com>

Sucuri CloudProxy is a well known Web Application Firewall (WAF) that 
caches content leading to quicker response times and less load for your 
website.

The CloudProxy cache can be cleared by visiting the Sucuri Dashboard or 
more conveniently, through the use of an API.  This module allows the 
clearing of the CloudProxy cache through the Drupal website itself.


REQUIREMENTS
------------

* An active CloudProxy license
* An A-Record pointing to the CloudProxy WAF
* The Site Key and Secret Key provided by Sucuri
* The PHP CURL library must be installed and enabled


RECOMMENDED MODULES
-------------------

There are no additional recommended modules.


INSTALLATION
------------
 
 * Install as you would normally install a contributed Drupal module. See:
   https://drupal.org/documentation/install/modules-themes/modules-7
   for further information.


CONFIGURATION
-------------

* Navigate to Configuration -> System -> Sucuri Cloud Proxy
* Enter the Site Key and Secret Key provided by Sucuri
* Click 'Save configuration'


TROUBLESHOOTING
---------------

* If CURL is not installed, please contact your hosting provider.
* If the Site Key or Secret Key fail to save, ensure they are correct.
* If Sucuri reports an incorrect key, ensure they were copied correctly.
* If your content does not update right away, give it a few minutes.

FAQ
---


MAINTAINERS
-----------

Current maintainers:
 * Luke Leber - https://www.drupal.org/user/3509746
