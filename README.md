Description
-----------
Note:
This module is alpha version.

Field API:
- Created new field type and added default field_formatter.
- Vimeo Thumbnail image will be saved locally.
- All data will be stored in core field_data_{field} table.
- No aditional tables was created.
- Added very basic fucntionality to play video, without any control settings.

Ctools plugin:
- New plugin support any types of CT and fields. And provide you possbility to render randomly any nodes based on specified field.



Requirements
------------
Drupal 7.x

Installation
------------
1. Copy module directory to the Drupal sites/all/modules || sites/{profile} directory.

2. Login as an administrator. Enable the module in the "Administer" -> "Modules"

3. Add new Vimeo field in your content type

4. (Optional) Update settings for field display mode.


To use Ctools plugin:

1. Open your panel page.

2. Add new plugin (For plugin was crteated own Vimeo group)

3. Select CT and field that will be used for featch random results.

4. (Optional) Select display mode to render you nodes.


Author
------
Alexei Goja
ag@peytz.dk
