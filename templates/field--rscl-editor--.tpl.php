<span class="<?php print $classes; ?>"<?php print $attributes; ?>>
  <?php
  $editors = array();
  foreach ($items as $delta => $item) {
    $editors[] = render($item);
  }
  print implode(',&nbsp;',$editors);
  ?>
</span>
