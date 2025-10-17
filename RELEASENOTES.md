v1.2.2
------
<em>Release date: October 17, 2025</em><br/>
<em>Changes since: <strong>v1.2.1</strong></em><br/>

This patch release for pgModeler 1.2.x brings the following improvements and fixes:

* [New] Added initial support for PostgreSQL 18.
* [Change] Refactored the result set handling, avoiding potential memory leaks.
* [Fix] Fixed a bug in XML parsing that was not configuring the DTD file path correctly.
* [Fix] Fixed a bug that was causing the collation object assigned to a column not to be reflected in SQL and XML codes.
* [Fix] Fixed a bug that was causing collation objects to be saved always as deterministic, even when the user unchecked the option in the editing form.
* [Fix] Fixed the database server version parsing routine so the default version can be used as a fallback when a major server version is not yet recognized by the tool.
