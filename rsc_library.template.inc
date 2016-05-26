<?php

/**
 * Implements hook_preprocess_HOOK(&$variables)
 * https://api.drupal.org/api/drupal/modules!system!theme.api.php/function/hook_preprocess_HOOK/7
 * @param $variables array() The variables for the node template.
 */
function rsc_library_preprocess_node(&$variables) {

  /*
   * For library articles:
   */

  if ($variables['type'] == 'rsc_library_article') {

    // add microdata
    $variables['attributes_array']['itemscope'] = NULL;
    $variables['attributes_array']['itemtype'] = 'http://schema.org/Article';
    $variables['title_attributes_array']['itemprop'] = 'name';

  }

  /*
   * For all rsc_library content types:
   */

  if (in_array($variables['type'], rsc_library_content_types())) {

    // find and render difficulty and author fields and merge them into a list:
    $article_infos = array();
    foreach ($variables['content'] as $field_name => &$field) {
      if (substr($field_name, 0, 12) === RSC_LIBRARY_PREFIX.'_author_') {
        $article_infos[1] = render($field);
        unset($variables['content'][$field_name]);
      } else {
        if (substr($field_name, 0, 16) === RSC_LIBRARY_PREFIX.'_difficulty_') {
          $article_infos[0] = render($field);
          unset($variables['content'][$field_name]);
        }
      }
    }
    ksort($article_infos);
    if (!empty($article_infos)) {
      $variables['content']['article_info'] = array(
        '#weight' => -20,
        '#markup' => '<ul class="article-info"><li class="first">' . implode('</li> <li>', $article_infos) . '</li></ul>',
      );
    }

    // For full view mode:
    if ($variables['view_mode'] == 'full') {

      // create compound field from source, date and pages fields
      $src = array();
      $pages = "";
      // find and render source field
      foreach($variables['content'] as $field_name => &$field) {
        if (substr($field_name,0,12) === RSC_LIBRARY_PREFIX.'_source_') {
          if (!empty($field[0])) {
            $src[] = t('Source: !src',array('!src'=>trim(render($field))));
            unset($variables['content'][$field_name]);
          }
          break; // there is only one
        }
      }
      if (!empty($c[RSC_LIBRARY_PREFIX.'_date'][0])) {
        $src[] = trim(render($c[RSC_LIBRARY_PREFIX.'_date']));
        unset($c[RSC_LIBRARY_PREFIX.'_date']);
      }
      if (!empty($c[RSC_LIBRARY_PREFIX.'_pages'][0])) {
        $pages = t('!pages&nbsp;pages.',array('!pages'=>trim(render($c[RSC_LIBRARY_PREFIX.'_pages'][0]))));
        unset($c[RSC_LIBRARY_PREFIX.'_pages']);
      }
      if ($src && $pages) {
        $c['source_info'] = array(
          '#weight' => -10,
          '#markup' => implode(', ',$src).". ".$pages,
        );
      } else if ($src) {
        $c['source_info'] = array(
          '#weight' => -10,
          '#markup' => implode(', ',$src),
        );
      } else if ($pages) {
        $c['source_info'] = array(
          '#weight' => -10,
          '#markup' => $pages,
        );
      }

      // make a list of fields (better semantics than a lot of divs)
      $exclude = array('links','comments',RSC_LIBRARY_PREFIX.'_body','article_info',RSC_LIBRARY_PREFIX.'_link',RSC_LIBRARY_PREFIX.'_attachment','field_image');
      $items = array();
      foreach(array_diff(array_keys($c),$exclude) as $key) {
        $field = &$c[$key];
        $items[$field['#weight']] = render($field);
        unset($field);
      }
      ksort($items);
      if (!empty($items)) {
        $c['fields-list'] = array(
          '#weight' => 4,
          '#markup' => '<ul class="fields-list"><li class="first">' . implode('</li><li>', $items) . '</li></ul>',
        );
      }

    }

  }

  /*
   * For rscl_block view mode:
   */

  if ($variables['view_mode'] == RSC_LIBRARY_PREFIX.'_block') {
    // use template node--rsc-block.tpl.php
    $variables['theme_hook_suggestions'][] = "node__{$variables['view_mode']}";
  }

}


/**
 * Implements hook_preprocess_HOOK(&$variables)
 * https://api.drupal.org/api/drupal/modules!system!theme.api.php/function/hook_preprocess_HOOK/7
 * @param $variables array() The variables for the field template.
 */
function rsc_library_preprocess_field(&$variables) {

  /* Modify the display of some fields */
  if ($variables['element']['#field_name'] == RSC_LIBRARY_PREFIX.'_copyright_notice') {
    // assume there is only one field value
    $variables['items'][0]['#prefix'] = t("This material is made available with the permission of ");
    $variables['items'][0]['#suffix'] = t(".");
  }

// TODO: improve this bit on microdata, or move it to a separate module?
  if ($variables['element']['#bundle'] == 'rsc_library_article') {

    // microdata
    // TODO: auto-detect field-names or make them manually configurable
    switch ($variables['element']['#field_name']) {

      case 'body':
        $variables['attributes_array']['itemprop'] = 'description';
        foreach($variables['items'] as &$item) {
          $item = preg_replace('/<a/','<a itemprop="about"',$item);
        }
        break;

      case 'field_author_tax':
        foreach ($variables['items'] as &$item) {
          $item['#options']['attributes']['itemprop'] = 'author';
        }
        break;

      case 'field_link':
        foreach ($variables['items'] as &$item) {
          $item['#markup'] = preg_replace('/<a/','$0 itemprop="url"',$item['#markup']);
        }
        break;

      case 'field_attachment':
        foreach ($variables['items'] as &$item) {
          $item['#markup'] = preg_replace('/<li class="read"><a/','$0 itemprop="url"',$item['#markup']);
        }
        break;

      case 'field_source_tax':
        $variables['attributes_array']['itemprop'] = 'publisher';
        break;

      case 'field_date':
        $variables['attributes_array']['itemprop'] = 'datePublished';
        break;
    }
  }

}