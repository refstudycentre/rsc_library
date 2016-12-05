<?php
// This is for backward compatibility.
// TODO: Remove this once content has been fully migrated to the rsc_library_link type, and the rscl-link field has been removed from the rsc_library_article type.
?>
<div class="<?php print $classes; ?>"<?php print $attributes; ?>><?php print render($items[0]); ?></div>