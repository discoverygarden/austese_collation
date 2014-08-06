<?php

/**
 * @file
 * Emicdora modifications.
 */
function get_edited_collation($document_id) {
  global $base_url;
  $calliope_url = 'html/comparison';
  $path = "$base_url/$calliope_url/$document_id";
  $url = url($path, array('query' => array(
      '_dc' => $_GET['_dc'],
      'version1' => $_GET['version1'],
      'version2' => $_GET['version2'],
      'diff_kind' => $_GET['diff_kind'],
  )));
  drupal_goto($url);
}

